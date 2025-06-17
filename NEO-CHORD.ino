// NEOCHORD MIDI Chord Keypad
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL343.h>
#include <Adafruit_NeoTrellisM4.h>

#define MIDI_CHANNEL     0
#define FIRST_MIDI_NOTE  36
#define BASE_OCTAVE      3
#define OCTAVE_MIN       1
#define OCTAVE_MAX       6
#define MAX_FAVORITES    28
#define CLEAR_HOLD_TIME  3000
#define STRUM_DELAY      75

Adafruit_ADXL343 accel = Adafruit_ADXL343(123, &Wire1);

enum Mode { GLOBAL_CHORD_MODE, FAVORITE_MODE, CIRCLE_FIFTHS_MODE };
Mode currentMode = GLOBAL_CHORD_MODE;

// Mode 3 Circle Key States
enum CircleKeyState { 
  ORANGE_STATE,  // C-G-D-A
  PINK_STATE,    // E-B-F#-C#
  RED_STATE,     // Ab-Eb-Bb-F 
};
CircleKeyState circleKeyState = ORANGE_STATE;

// Button state tracking
unsigned long lastModeChange = 0;
bool modeLedState = false;
unsigned long button8PressTime = 0;
bool button8Held = false;
unsigned long button16PressTime = 0;
bool button16Held = false;
unsigned long button24PressTime = 0;
bool button24Held = false;

// Mode 1 variables
int rowChordTypes[4] = {0, 0, 0, 0}; // Chord types for each row (0-11)
int octaveShift = 0; // 0=octaves 3-4, 1=5-6, 2=1-2

// Mode 3 variables
int currentKeyIndex = 0;
int circleOctave = BASE_OCTAVE;

// Favorite chords variables
struct SavedChord {
  bool saved = false;
  int rootNote;
  int type;
};
SavedChord favoriteChords[MAX_FAVORITES];
SavedChord copiedRow[7];
int copiedRowKeyIndex = 0;
bool octaveShiftModeActive = false;  // Tracks if we're in octave shift mode
int favoriteOctaveShift = 0;         // Current octave shift for favorites
int lastPlayedFavorite = -1;         // Last played favorite index

// Strumming variables
unsigned long nextStrumTime = 0;
bool isStrumming = false;
int currentStrumNote = 0;
int strumNotes[20]; // Enough for 5-note chords across 4 octaves
int strumNoteCount = 0;

const char* KEY_NAMES[] = {
  "C", "G", "D", "A", "E", "B", "F#", "C#", "Ab", "Eb", "Bb", "F"};
const int KEY_ROOTS[] = {0, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10, 5};
const int CIRCLE_CHORD_TYPES[] = {0, 0, 0, 1, 1, 1, 2}; // I, IV, V, ii, iii, vi, vii°
const int CIRCLE_CHORD_OFFSETS[] = {0, 5, 7, 2, 4, 9, 11};

bool saveModeActive = false;
bool deleteModeActive = false;
bool strumModeActive = false;
bool rowCopyModeActive = false;
int selectedChordForSave = -1;
int selectedChordType = 0;
int selectedRootNote = 0;
bool chordReadyForSave = false;

unsigned long rowCopyPulseTime = 0;
bool rowCopyPulseState = false;

enum StrumMode { STRUM_UP, STRUM_DOWN };
StrumMode strumMode = STRUM_UP;

int xCC = 1;
int last_xbend = 0;
int last_ybend = 0;

const int UTILITY_BUTTONS[] = {0, 8, 16, 24};
const uint32_t UTILITY_COLOR = 0xFFA500; // Orange
const uint32_t PINK_COLOR = 0xFF69B4;
const uint32_t TEAL_COLOR = 0x008080;
const uint32_t STRUM_UP_COLOR = 0xFF1493;
const uint32_t STRUM_DOWN_COLOR = 0x008080;
const int MODE_TOGGLE_BUTTON = 0;
const int SAVE_BUTTON = 8;
const int SHIFT_BUTTON = 16;
const int CHORD_CYCLE_BUTTON = 24;

Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();

// Chord definitions
const int MAJOR_CHORD[] = {0, 4, 7};
const int MINOR_CHORD[] = {0, 3, 7};
const int DIM_CHORD[] = {0, 3, 6};
const int AUG_CHORD[] = {0, 4, 8};
const int SUS4_CHORD[] = {0, 5, 7};
const int ADD9_CHORD[] = {0, 4, 7, 14};
const int SEVENTH_CHORD[] = {0, 4, 7, 10};
const int MAJ7_CHORD[] = {0, 4, 7, 11};
const int MIN7_CHORD[] = {0, 3, 7, 10};
const int NINTH_CHORD[] = {0, 4, 7, 10, 14};
const int MAJ9_CHORD[] = {0, 4, 7, 11, 14};
const int MIN9_CHORD[] = {0, 3, 7, 10, 14};

const uint32_t CHORD_COLORS[] = {
  trellis.Color(0, 127, 0),     // Major - Green
  trellis.Color(0, 0, 127),     // Minor - Blue
  trellis.Color(127, 0, 0),     // Diminished - Red
  trellis.Color(127, 127, 0),   // Augmented - Yellow
  trellis.Color(189, 158, 61),   // SUS4 - Purple
  trellis.Color(255, 105, 180), // ADD9 - Pink
  trellis.Color(255, 155, 0),   // 7th - Magenta
  trellis.Color(78, 107, 0),   // MAJ7 - Teal
  trellis.Color(0, 234, 255), // MIN7 - Lavender
  trellis.Color(126, 0, 255),   // NINTH - Cyan
  trellis.Color(91, 255, 143),     // MAJ9 - Maroon
  trellis.Color(86, 96, 212)      // MIN9 - Lime
};

void setup() {
  Serial.begin(115200);
  Serial.println("MIDI Chord Keypad Initializing...");
    
  trellis.begin();
  trellis.setBrightness(5);

  trellis.enableUSBMIDI(true);
  trellis.setUSBMIDIchannel(MIDI_CHANNEL);
  
  trellis.enableUARTMIDI(true);
  trellis.setUARTMIDIchannel(MIDI_CHANNEL);
  
  if(!accel.begin()) {
    Serial.println("No accelerometer found");
    while(1);
  }

  initializeButtonColors();
}

void playChord(int rootNote, int type, bool noteOn) {
  int velocity = noteOn ? 64 : 0;
  int noteCount = 3; // Default for most chord types
  
  // Set note count based on chord type
  switch(type) {
    case 5: case 6: case 7: case 8:
      noteCount = 4;
      break;
    case 9: case 10: case 11:
      noteCount = 5;
      break;
  }
  
  for (int i = 0; i < noteCount; i++) {
    int note = rootNote + getChordNote(i, type);
    noteOn ? trellis.noteOn(note, velocity) : trellis.noteOff(note, velocity);
  }
}

void strumChord(int rootNote, int type, bool noteOn) {
  if (!noteOn) {
    // Turn off all strummed notes
    for (int i = 0; i < strumNoteCount; i++) {
      trellis.noteOff(strumNotes[i], 64);
    }
    isStrumming = false;
    return;
  }

  // Determine note count based on chord type
  int noteCount;
  switch(type) {
    case 9: case 10: case 11: // 9th chords have 5 notes
      noteCount = 5;
      break;
    case 5: case 6: case 7: case 8: // 7th chords have 4 notes
      noteCount = 4;
      break;
    default: // Other chords have 3 notes
      noteCount = 3;
      break;
  }

  // Calculate total notes to strum (4 octaves worth)
  strumNoteCount = noteCount * 4;
  
  // Generate all notes to be strummed
  if (strumMode == STRUM_UP) {
    // Strum up: low to high
    for (int octave = 0; octave < 4; octave++) {
      for (int i = 0; i < noteCount; i++) {
        strumNotes[octave * noteCount + i] = rootNote + getChordNote(i, type) + (octave * 12);
      }
    }
  } else {
    // Strum down: high to low
    for (int octave = 3; octave >= 0; octave--) {
      for (int i = noteCount-1; i >= 0; i--) {
        strumNotes[(3-octave) * noteCount + (noteCount-1-i)] = rootNote + getChordNote(i, type) + (octave * 12);
      }
    }
  }

  // Initialize strumming
  currentStrumNote = 0;
  nextStrumTime = millis();
  isStrumming = true;
  
  // Play the first note immediately
  trellis.noteOn(strumNotes[currentStrumNote], 64);
  currentStrumNote++;
}

void updateStrumming() {
  if (!isStrumming) return;
  
  unsigned long currentTime = millis();
  if (currentTime >= nextStrumTime) {
    // Turn off previous note
    if (currentStrumNote > 0) {
      trellis.noteOff(strumNotes[currentStrumNote-1], 64);
    }
    
    // Play next note if available
    if (currentStrumNote < strumNoteCount) {
      trellis.noteOn(strumNotes[currentStrumNote], 64);
      currentStrumNote++;
      nextStrumTime = currentTime + STRUM_DELAY;
    } else {
      // Strumming complete
      isStrumming = false;
    }
  }
}

int getChordNote(int index, int type) {
  switch(type) {
    case 0: return MAJOR_CHORD[index % 3];
    case 1: return MINOR_CHORD[index % 3];
    case 2: return DIM_CHORD[index % 3];
    case 3: return AUG_CHORD[index % 3];
    case 4: return SUS4_CHORD[index % 3];
    case 5: return ADD9_CHORD[index % 4];
    case 6: return SEVENTH_CHORD[index % 4];
    case 7: return MAJ7_CHORD[index % 4];
    case 8: return MIN7_CHORD[index % 4];
    case 9: return NINTH_CHORD[index % 5];
    case 10: return MAJ9_CHORD[index % 5];
    case 11: return MIN9_CHORD[index % 5];
    default: return MAJOR_CHORD[index % 3];
  }
}

bool isUtilityButton(int key) {
  for (int i = 0; i < 4; i++) {
    if (key == UTILITY_BUTTONS[i]) {
      return true;
    }
  }
  return false;
}

bool isRowTypeButton(int key) {
  return (currentMode == GLOBAL_CHORD_MODE) && (key == 7 || key == 15 || key == 23 || key == 31);
}

int getFavoriteButtonIndex(int key) {
  if (key < 8) return key - 1;
  else if (key < 16) return key - 2;
  else if (key < 24) return key - 3;
  else return key - 4;
}

void initializeButtonColors() {
  for (int i = 0; i < 32; i++) {
    if (isUtilityButton(i)) {
      if (i == MODE_TOGGLE_BUTTON) {
        trellis.setPixelColor(i, 0xFFFFFF); // White for mode toggle
      } 
      else if (i == SHIFT_BUTTON) {
        if (currentMode == FAVORITE_MODE) {
          if (strumModeActive && !octaveShiftModeActive) {
            trellis.setPixelColor(i, strumMode == STRUM_UP ? STRUM_UP_COLOR : STRUM_DOWN_COLOR);
          } else if (octaveShiftModeActive) {
            trellis.setPixelColor(i, 0xFFFFFF); // White when in octave shift mode
          } else {
            trellis.setPixelColor(i, UTILITY_COLOR); // Orange by default
          }
        } else {
          trellis.setPixelColor(i, UTILITY_COLOR); // Orange for other modes
        }
      }
      else if (i == CHORD_CYCLE_BUTTON) {
        // Preserve the button color based on circleKeyState in CIRCLE_FIFTHS_MODE
        if (currentMode == CIRCLE_FIFTHS_MODE) {
          switch(circleKeyState) {
            case ORANGE_STATE:
              trellis.setPixelColor(i, UTILITY_COLOR); // Orange
              break;
            case PINK_STATE:
              trellis.setPixelColor(i, PINK_COLOR); // Pink
              break;
            case RED_STATE:
              trellis.setPixelColor(i, TEAL_COLOR); // Teal
              break;
          }
        } else {
          trellis.setPixelColor(i, UTILITY_COLOR); // Orange for other modes
        }
      }
      else if (i == SAVE_BUTTON && rowCopyModeActive) {
        trellis.setPixelColor(i, copiedRow[0].saved ? 0x00FF00 : 0xFF0000);
      } 
      else {
        trellis.setPixelColor(i, UTILITY_COLOR); // Orange by default
      }
    } 
    else if (isRowTypeButton(i)) {
      trellis.setPixelColor(i, UTILITY_COLOR);
    }
    else if (currentMode == GLOBAL_CHORD_MODE) {
      int row = i / 8;
      trellis.setPixelColor(i, CHORD_COLORS[rowChordTypes[row]]);
    } 
    else if (currentMode == FAVORITE_MODE) {
      int favIndex = getFavoriteButtonIndex(i);
      trellis.setPixelColor(i, favoriteChords[favIndex].saved ? CHORD_COLORS[favoriteChords[favIndex].type] : 0);
    } 
    else if (currentMode == CIRCLE_FIFTHS_MODE) {
      int col = (i % 8) - 1;
      if (col >= 0 && col < 7) {
        trellis.setPixelColor(i, CHORD_COLORS[CIRCLE_CHORD_TYPES[col]]);
      }else if (currentMode == FAVORITE_MODE && octaveShiftModeActive) {
    trellis.setPixelColor(i, UTILITY_COLOR); // Stay orange in octave shift mode
     } else {
    trellis.setPixelColor(i, UTILITY_COLOR); // Orange for other modes
  }
}
    }
  
  updateUtilityButtonStates();
}

void shiftCircleKeys() {
  circleKeyState = (CircleKeyState)((circleKeyState + 1) % 3);
  
  switch(circleKeyState) {
    case ORANGE_STATE:
      currentKeyIndex = 0;
      Serial.println("Circle keys: C-G-D-A (Orange)");
      trellis.setPixelColor(CHORD_CYCLE_BUTTON, UTILITY_COLOR);
      break;
    case PINK_STATE:
      currentKeyIndex = 4;
      Serial.println("Circle keys: E-B-F#-C# (Pink)");
      trellis.setPixelColor(CHORD_CYCLE_BUTTON, PINK_COLOR);
      break;
    case RED_STATE:
      currentKeyIndex = 8;
      Serial.println("Circle keys: F-Bb-Eb-Ab (TEAL)");
      trellis.setPixelColor(CHORD_CYCLE_BUTTON, TEAL_COLOR);
      break;
  }
  
  initializeButtonColors();
}

void changeRowChordType(int row) {
  rowChordTypes[row] = (rowChordTypes[row] + 1) % 12;
  
  Serial.print("Row ");
  Serial.print(row);
  Serial.print(" chord type: ");
  printChordType(rowChordTypes[row]);
  Serial.println();
  initializeButtonColors();
}

void changeAllChordTypes() {
  int newType = (rowChordTypes[0] + 1) % 12;
  
  for (int i = 0; i < 4; i++) {
    rowChordTypes[i] = newType;
  }
  
  Serial.print("All rows chord type: ");
  printChordType(newType);
  Serial.println();
  initializeButtonColors();
}

void toggleOctaveShift() {
  octaveShift = (octaveShift + 1) % 3;
  Serial.print("Octave shift: ");
  switch(octaveShift) {
    case 0: Serial.println("3-4 (default)"); break;
    case 1: Serial.println("5-6"); break;
    case 2: Serial.println("1-2"); break;
  }
  initializeButtonColors();
}

void toggleStrumMode() {
  strumMode = (strumMode == STRUM_UP) ? STRUM_DOWN : STRUM_UP;
  Serial.print("Strum: ");
  Serial.println(strumMode == STRUM_UP ? "UP" : "DOWN");
  initializeButtonColors();
}

void toggleMode() {
  if (chordReadyForSave && (currentMode + 1) % 3 == FAVORITE_MODE) {
    saveModeActive = true;
    Serial.println("Switched with chord ready to save");
  } else if (currentMode == FAVORITE_MODE) {
    saveModeActive = false;
    deleteModeActive = false;
    octaveShiftModeActive = false; // Exit octave shift mode when changing modes
  }
  
  currentMode = (Mode)((currentMode + 1) % 3);
  
  Serial.print("Mode: ");
  switch(currentMode) {
    case GLOBAL_CHORD_MODE: Serial.println("Global"); break;
    case FAVORITE_MODE: Serial.println("Favorites"); break;
    case CIRCLE_FIFTHS_MODE: Serial.println("Circle of 5ths"); break;
  }
  
  // Only update the CHORD_CYCLE_BUTTON if we're in CIRCLE_FIFTHS_MODE
  if (currentMode == CIRCLE_FIFTHS_MODE) {
    switch(circleKeyState) {
      case ORANGE_STATE:
        trellis.setPixelColor(CHORD_CYCLE_BUTTON, UTILITY_COLOR);
        break;
      case PINK_STATE:
        trellis.setPixelColor(CHORD_CYCLE_BUTTON, PINK_COLOR);
        break;
      case RED_STATE:
        trellis.setPixelColor(CHORD_CYCLE_BUTTON, TEAL_COLOR);
        break;
    }
  } else {
    trellis.setPixelColor(CHORD_CYCLE_BUTTON, UTILITY_COLOR);
  }
  
  initializeButtonColors();
}

void changeCircleOctave() {
  circleOctave = (circleOctave + 1) % (OCTAVE_MAX + 1);
  if (circleOctave < OCTAVE_MIN) circleOctave = OCTAVE_MIN;
  Serial.print("Octave: ");
  Serial.println(circleOctave);
}

void toggleSaveMode() {
  if (currentMode != FAVORITE_MODE) {
    if (saveModeActive && selectedChordForSave != -1) {
      selectedChordForSave = -1;
      chordReadyForSave = false;
      Serial.println("Deselected chord");
    } else {
      saveModeActive = !saveModeActive;
      Serial.println(saveModeActive ? "Save mode ON" : "Save mode OFF");
    }
  } else {
    deleteModeActive = !deleteModeActive;
    Serial.println(deleteModeActive ? "Delete mode ON" : "Delete mode OFF");
  }
  updateUtilityButtonStates();
}

void saveChordToFavorite(int favIndex) {
  favoriteChords[favIndex].saved = true;
  favoriteChords[favIndex].rootNote = selectedRootNote;
  favoriteChords[favIndex].type = selectedChordType;
  
  Serial.print("Saved to slot ");
  Serial.print(favIndex);
  Serial.print(": ");
  printChordType(selectedChordType);
  Serial.print(" @ ");
  Serial.println(selectedRootNote);
  
  saveModeActive = false;
  chordReadyForSave = false;
  selectedChordForSave = -1;
  initializeButtonColors();
}

void clearAllFavorites() {
  for (int i = 0; i < MAX_FAVORITES; i++) {
    favoriteChords[i].saved = false;
  }
  Serial.println("Cleared all favorites");
  initializeButtonColors();
}

void updateUtilityButtonStates() {
  if (saveModeActive) {
    if (selectedChordForSave != -1) {
      unsigned long currentMillis = millis();
      if (currentMillis - lastModeChange >= 500) {
        lastModeChange = currentMillis;
        modeLedState = !modeLedState;
        trellis.setPixelColor(SAVE_BUTTON, modeLedState ? CHORD_COLORS[selectedChordType] : 0);
      }
    } else {
      trellis.setPixelColor(SAVE_BUTTON, 0xFFFFFF);
    }
  } else if (deleteModeActive) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastModeChange >= 500) {
      lastModeChange = currentMillis;
      modeLedState = !modeLedState;
      trellis.setPixelColor(SAVE_BUTTON, modeLedState ? 0xFFFFFF : 0);
    }
  } else if (rowCopyModeActive) {
    // Handled in initializeButtonColors
  } else {
    trellis.setPixelColor(SAVE_BUTTON, UTILITY_COLOR);
  }
  
  trellis.setPixelColor(MODE_TOGGLE_BUTTON, 0xFFFFFF);
  
  if (currentMode == FAVORITE_MODE) {
    if (strumModeActive && !octaveShiftModeActive) {
      trellis.setPixelColor(SHIFT_BUTTON, strumMode == STRUM_UP ? STRUM_UP_COLOR : STRUM_DOWN_COLOR);
    }
  }
}

void updateRowCopyMode() {
  if (rowCopyModeActive) {
    unsigned long currentTime = millis();
    if (currentTime - rowCopyPulseTime >= 200) {
      rowCopyPulseTime = currentTime;
      rowCopyPulseState = !rowCopyPulseState;
      trellis.setPixelColor(SAVE_BUTTON, rowCopyPulseState ? (copiedRow[0].saved ? 0xB1CCB1 : 0xFFA500) : 0);
    }
  }
}

void handleUtilityButton(int key, bool pressed) {
  if (!pressed) {
    if (key == CHORD_CYCLE_BUTTON) {
      button24Held = false;
    } else if (key == SAVE_BUTTON) {
      button8Held = false;
      if (!rowCopyModeActive) {
        trellis.setPixelColor(key, UTILITY_COLOR);
      }
    } else if (key == SHIFT_BUTTON) {
      if (!(currentMode == FAVORITE_MODE && (strumModeActive || octaveShiftModeActive))) {
        trellis.setPixelColor(key, UTILITY_COLOR);
      }
    }
    return;
  }

if (key == CHORD_CYCLE_BUTTON) {
  button24PressTime = millis();
  button24Held = true;
  
  if (currentMode == FAVORITE_MODE && octaveShiftModeActive) {
    // In octave shift mode - keep button orange (don't change color)
    trellis.setPixelColor(key, UTILITY_COLOR);
  } else {
    // Normal behavior - flash white when pressed
    trellis.setPixelColor(key, 0xFFFFFF);
  }
  
  if (currentMode == GLOBAL_CHORD_MODE) {
    changeAllChordTypes();
  } else if (currentMode == FAVORITE_MODE) {
    if (!octaveShiftModeActive) { // Only allow strum mode when not in octave shift mode
      strumModeActive = !strumModeActive;
      Serial.println(strumModeActive ? "Strum ON" : "Strum OFF");
      initializeButtonColors();
    }
  } else {
    shiftCircleKeys();
  }
}
  else if (key == SAVE_BUTTON) {
    button8PressTime = millis();
    button8Held = true;
    
    if (currentMode == FAVORITE_MODE) {
      trellis.setPixelColor(key, 0xFFFFFF);
      if ((millis() - button8PressTime) >= CLEAR_HOLD_TIME) {
        clearAllFavorites();
        button8Held = false;
      } else {
        toggleSaveMode();
      }
    } 
    else if (currentMode == CIRCLE_FIFTHS_MODE) {
      rowCopyModeActive = !rowCopyModeActive;
      if (rowCopyModeActive) {
        rowCopyPulseTime = millis();
        Serial.println("Row copy mode ON");
      } else {
        Serial.println("Row copy mode OFF");
        trellis.setPixelColor(key, UTILITY_COLOR);
      }
    } 
    else {
      trellis.setPixelColor(key, 0xFFFFFF);
      toggleSaveMode();
    }
  } 
  else if (key == SHIFT_BUTTON) {
    button16PressTime = millis();
    button16Held = true;
    trellis.setPixelColor(key, 0xFFFFFF);
    
    if (currentMode == GLOBAL_CHORD_MODE) {
      toggleOctaveShift();
    } else if (currentMode == FAVORITE_MODE) {
      if (strumModeActive && !octaveShiftModeActive) {
        toggleStrumMode();
      } else {
        // Toggle octave shift mode for favorites
        octaveShiftModeActive = !octaveShiftModeActive;
        if (octaveShiftModeActive) {
          Serial.println("Favorite octave shift ON");
        } else {
          // Save the octave shift if we have a last played favorite
          if (lastPlayedFavorite != -1) {
            favoriteChords[lastPlayedFavorite].rootNote += favoriteOctaveShift * 12;
            favoriteOctaveShift = 0;
            lastPlayedFavorite = -1;
            Serial.println("Saved octave change to favorite");
          }
          Serial.println("Favorite octave shift OFF");
        }
        initializeButtonColors();
      }
    } else if (currentMode == CIRCLE_FIFTHS_MODE) {
      changeCircleOctave();
    }
  } 
  else if (key == MODE_TOGGLE_BUTTON) {
    trellis.setPixelColor(key, 0xFFFFFF);
    toggleMode();
  }
}

void handleMode1ChordButton(int key, bool pressed) {
  if (isRowTypeButton(key)) {
    if (pressed) {
      int row = key / 8;
      changeRowChordType(row);
      trellis.setPixelColor(key, 0xFFFFFF);
    }
    return;
  }
  
  if (key % 8 == 0 || key % 8 == 7) {
    return; 
  }

  int row = key / 8;
  int col = (key % 8) - 1;
  
  int chromaticOffset = col + (row % 2) * 6;
  
  int baseOctave;
  switch(octaveShift) {
    case 0: baseOctave = (row < 2) ? 3 : 4; break;
    case 1: baseOctave = (row < 2) ? 5 : 6; break;
    case 2: baseOctave = (row < 2) ? 1 : 2; break;
  }
  
  int rootNote = 48 + (baseOctave - 3) * 12 + (chromaticOffset % 12);
  int chordType = rowChordTypes[row];
  
  if (pressed) {
    if (saveModeActive) {
      selectedChordForSave = key;
      selectedChordType = chordType;
      selectedRootNote = rootNote;
      chordReadyForSave = true;
      Serial.print("Selected: ");
      printChordType(chordType);
      Serial.print(" @ ");
      Serial.println(rootNote);
      trellis.setPixelColor(key, 0xFFFFFF);
      
      // Automatically switch to Favorites mode
      currentMode = FAVORITE_MODE;
      initializeButtonColors();
    } else {
      Serial.print("Playing: ");
      printChordType(chordType);
      Serial.print(" @ ");
      Serial.println(rootNote);
      trellis.setPixelColor(key, 0xFFFFFF);
      playChord(rootNote, chordType, true);
    }
  } else {
    if (!saveModeActive) {
      trellis.setPixelColor(key, CHORD_COLORS[chordType]);
      playChord(rootNote, chordType, false);
    }
  }
}

void handleMode2FavoriteButton(int key, bool pressed) {
  int favIndex = getFavoriteButtonIndex(key);
  int row = key / 8;
  
  if (pressed) {
    if (rowCopyModeActive) {
      if (copiedRow[0].saved) {
        int startIndex = row * 7;
        for (int i = 0; i < 7; i++) {
          if (startIndex + i < MAX_FAVORITES) {
            favoriteChords[startIndex + i] = copiedRow[i];
          }
        }
        
        Serial.print("Pasted row to favorites row ");
        Serial.println(row);
        
        rowCopyModeActive = false;
        trellis.setPixelColor(SAVE_BUTTON, UTILITY_COLOR);
        initializeButtonColors();
      }
    } 
    else if (saveModeActive && chordReadyForSave) {
      if (!favoriteChords[favIndex].saved) {
        saveChordToFavorite(favIndex);
      }
    } 
    else if (deleteModeActive) {
      if (favoriteChords[favIndex].saved) {
        favoriteChords[favIndex].saved = false;
        Serial.print("Deleted slot ");
        Serial.println(favIndex);
        trellis.setPixelColor(key, 0);
        deleteModeActive = false;
        saveModeActive = false;
        chordReadyForSave = false;
        updateUtilityButtonStates();
      }
    }
    else if (favoriteChords[favIndex].saved) {
      Serial.print("Playing favorite ");
      Serial.print(favIndex);
      Serial.print(": ");
      printChordType(favoriteChords[favIndex].type);
      Serial.print(" @ ");
      
      // Apply octave shift if in octave shift mode
      int playedOctave = favoriteChords[favIndex].rootNote / 12;
      int playedNote = favoriteChords[favIndex].rootNote % 12;
      
      if (octaveShiftModeActive) {
        favoriteOctaveShift = (favoriteOctaveShift + 1) % 3; // Cycle through 0, +1, -1
        if (favoriteOctaveShift == 2) favoriteOctaveShift = -1; // Make it 0, +1, -1
        lastPlayedFavorite = favIndex;
      }
      
      int actualNote = favoriteChords[favIndex].rootNote + (favoriteOctaveShift * 12);
      Serial.println(actualNote);
      
      // Stop any current strumming before starting new one
      if (isStrumming) {
        for (int i = 0; i < strumNoteCount; i++) {
          trellis.noteOff(strumNotes[i], 64);
        }
        isStrumming = false;
      }
      
      trellis.setPixelColor(key, 0xFFFFFF);
      
      if (strumModeActive && !octaveShiftModeActive) {
        strumChord(actualNote, favoriteChords[favIndex].type, true);
      } else {
        playChord(actualNote, favoriteChords[favIndex].type, true);
      }
    }
  } else {
    if (favoriteChords[favIndex].saved && !saveModeActive && !deleteModeActive) {
      int actualNote = favoriteChords[favIndex].rootNote + (favoriteOctaveShift * 12);
      trellis.setPixelColor(key, CHORD_COLORS[favoriteChords[favIndex].type]);
      
      if (strumModeActive && !octaveShiftModeActive) {
        strumChord(actualNote, favoriteChords[favIndex].type, false);
      } else {
        playChord(actualNote, favoriteChords[favIndex].type, false);
      }
    }
  }
}

void handleMode3CircleButton(int key, bool pressed) {
  if (key % 8 == 0) return;
  
  int row = (key / 8) % 7;
  int col = (key % 8) - 1;
  if (col < 0 || col >= 7) return;
  
  int shiftedIndex = (currentKeyIndex + row) % 15;
  int rootOffset = KEY_ROOTS[shiftedIndex];
  int chordOffset = CIRCLE_CHORD_OFFSETS[col];
  int chordType = CIRCLE_CHORD_TYPES[col];
  int rootNote = 60 + (rootOffset + chordOffset) % 12 + (circleOctave - 4) * 12;
  
  if (pressed) {
    if (rowCopyModeActive) {
      for (int i = 0; i < 7; i++) {
        copiedRow[i].saved = false;
      }
      
      for (int i = 0; i < 7; i++) {
        copiedRow[i].saved = true;
        copiedRow[i].rootNote = 60 + (rootOffset + CIRCLE_CHORD_OFFSETS[i]) % 12 + (circleOctave - 4) * 12;
        copiedRow[i].type = CIRCLE_CHORD_TYPES[i];
      }
      copiedRowKeyIndex = shiftedIndex;
      
      Serial.print("Copied row: ");
      Serial.println(KEY_NAMES[copiedRowKeyIndex]);
      
      for (int i = 1; i < 8; i++) {
        trellis.setPixelColor(row * 8 + i, 0xFFFFFF);
      }
      delay(200);
      for (int i = 1; i < 8; i++) {
        trellis.setPixelColor(row * 8 + i, CHORD_COLORS[CIRCLE_CHORD_TYPES[i-1]]);
      }
      
      currentMode = FAVORITE_MODE;
      initializeButtonColors();
    } 
    else if (saveModeActive) {
      selectedChordForSave = key;
      selectedChordType = chordType;
      selectedRootNote = rootNote;
      chordReadyForSave = true;
      
      Serial.print("Selected: ");
      switch(col) {
        case 0: Serial.print("I"); break;
        case 1: Serial.print("IV"); break;
        case 2: Serial.print("V"); break;
        case 3: Serial.print("ii"); break;
        case 4: Serial.print("iii"); break;
        case 5: Serial.print("vi"); break;
        case 6: Serial.print("vii°"); break;
      }
      Serial.print(" (");
      printChordType(chordType);
      Serial.print(") @ ");
      Serial.println(rootNote);
      
      trellis.setPixelColor(key, 0xFFFFFF);
    } else {
      Serial.print("Playing ");
      Serial.print(KEY_NAMES[shiftedIndex]);
      Serial.print(" ");
      switch(col) {
        case 0: Serial.print("I"); break;
        case 1: Serial.print("IV"); break;
        case 2: Serial.print("V"); break;
        case 3: Serial.print("ii"); break;
        case 4: Serial.print("iii"); break;
        case 5: Serial.print("vi"); break;
        case 6: Serial.print("vii°"); break;
      }
      Serial.print(" (");
      printChordType(chordType);
      Serial.print(") @ ");
      Serial.println(rootNote);
      
      trellis.setPixelColor(key, 0xFFFFFF);
      playChord(rootNote, chordType, true);
    }
  } else {
    if (!saveModeActive) {
      trellis.setPixelColor(key, CHORD_COLORS[chordType]);
      playChord(rootNote, chordType, false);
    }
  }
}

void loop() {
  trellis.tick();
  
  // Update row copy mode if active
  if (rowCopyModeActive) {
    updateRowCopyMode();
  }
  
  // Handle button 8 hold for FAVORITE_MODE (clear all favorites)
  if (button8Held && currentMode == FAVORITE_MODE) {
    if (millis() - button8PressTime >= CLEAR_HOLD_TIME) {
      clearAllFavorites();
      button8Held = false;
      trellis.setPixelColor(SAVE_BUTTON, UTILITY_COLOR);
    }
  }
  
  // Update strumming if active
  updateStrumming();

  // Update button states if needed
  if (saveModeActive || deleteModeActive || rowCopyModeActive || 
      (button8Held && currentMode == FAVORITE_MODE)) {
    updateUtilityButtonStates();
  }

  while (trellis.available()) {
    keypadEvent e = trellis.read();
    int key = e.bit.KEY;
    
    if (isUtilityButton(key)) {
      handleUtilityButton(key, e.bit.EVENT == KEY_JUST_PRESSED);
      continue;
    }

    switch(currentMode) {
      case GLOBAL_CHORD_MODE:
        handleMode1ChordButton(key, e.bit.EVENT == KEY_JUST_PRESSED);
        break;
      case FAVORITE_MODE:
        handleMode2FavoriteButton(key, e.bit.EVENT == KEY_JUST_PRESSED);
        break;
      case CIRCLE_FIFTHS_MODE:
        handleMode3CircleButton(key, e.bit.EVENT == KEY_JUST_PRESSED);
        break;
    }
  }

  // Handle accelerometer for pitch bend
  sensors_event_t event;
  accel.getEvent(&event);
  
  int xbend = 0;
  int ybend = 0;

  if (abs(event.acceleration.y) < 2.0) {
    ybend = 8192;
  } else {
    if (event.acceleration.y > 0) {
      ybend = ofMap(event.acceleration.y, 2.0, 10.0, 8192, 0, true);
    } else {
      ybend = ofMap(event.acceleration.y, -2.0, -10.0, 8192, 16383, true);
    }
  }
  if (ybend != last_ybend) {
    trellis.pitchBend(ybend);
    last_ybend = ybend;
  }

  if (abs(event.acceleration.x) < 2.0) {
    xbend = 0;
  } else {
    if (event.acceleration.x > 0) {
      xbend = ofMap(event.acceleration.x, 2.0, 10.0, 0, 127, true);
    } else {
      xbend = ofMap(event.acceleration.x, -2.0, -10.0, 0, 127, true);
    }
  }
  if (xbend != last_xbend) {
    trellis.controlChange(xCC, xbend);
    last_xbend = xbend;
  }

  trellis.sendMIDI();
  delay(10);
}

void printChordType(int type) {
  switch(type) {
    case 0: Serial.print("Major"); break;
    case 1: Serial.print("Minor"); break;
    case 2: Serial.print("Diminished"); break;
    case 3: Serial.print("Augmented"); break;
    case 4: Serial.print("SUS4"); break;
    case 5: Serial.print("ADD9"); break;
    case 6: Serial.print("7th"); break;
    case 7: Serial.print("MAJ7"); break;
    case 8: Serial.print("MIN7"); break;
    case 9: Serial.print("NINTH"); break;
    case 10: Serial.print("MAJ9"); break;
    case 11: Serial.print("MIN9"); break;
  }
}

float ofMap(float value, float inputMin, float inputMax, float outputMin, float outputMax, bool clamp) {
    float outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
    if (clamp) {
      if (outputMax < outputMin) {
        if (outVal < outputMax)  outVal = outputMax;
        else if (outVal > outputMin)  outVal = outputMin;
      } else {
        if (outVal > outputMax) outVal = outputMax;
        else if (outVal < outputMin)  outVal = outputMin;
      }
    }
    return outVal;
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return trellis.Color(127 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return trellis.Color(0, WheelPos * 3, 127 - WheelPos * 3);
  }
  WheelPos -= 170;
  return trellis.Color(WheelPos * 3, 127 - WheelPos * 3, 0);
}
