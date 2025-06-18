The NEO-CHORD is a USB-MIDI controller that plays full chords at the push of a button.  Written for the Adafruit NeoTrellisM4. 

Modes Overview
 
   Mode 1: Global Chords Mode (Chromatic Grid)
   
    * Layout: 4 rows × 6 columns (excluding utility buttons).
    * Each row is assigned a chord type (e.g., Major, Minor, 7th).
    * Columns represent semitone steps (chromatic scale).
    * Chord Selection:
    * Rows: Each row has a fixed chord type (configurable via "row type" buttons on the right).
      Example:

      Row 0: Major    
      Row 1: Minor
      Row 2: 7th
      Row 3: Suspended 4th

     * Columns: Buttons in a row play the same chord type transposed by semitones. E.g., Column 0 = C Major, Column 1 = C# Major, etc.
     * Octave Shifting:
     * Toggle between octave ranges (1-2, 3-4, 5-6) using the Shift button.

  Mode 2: Favorites Mode
  
         * Layout: 4 rows × 7 columns (28 slots total) to save/recall custom chords.
         * Saved chords are highlighted in their chord-type color.
 
  Features:

         * Save Chords:
            In Global Chord Mode or Circle of Fifths Mode, hold a chord button.
            Press Save (button 8) to enter save mode.
            Press an empty slot in Favorites Mode to save.
         
         * Delete Chords:
            Press Save to toggle delete mode, then press a saved chord.
      
         * Strumming: Toggle strumming (up/down) with the Chord Cycle button (button 24).
      
         * Octave Shift: Hold Shift to transpose saved chords ±1 octave.

  Mode 3: Circle of Fifths Mode
    
    Layout: 7 columns (one for each diatonic chord in a key) 4 rows (for different keys in the circle of fifths).

     * Chord Progressions:
     
          Columns: Represent chords in a key (I, IV, V, ii, iii, vi, vii°).
  
          Example in C Major:
  
          Column 0: C Major (I)
  
          Column 1: F Major (IV)
  
          Column 2: G Major (V)
  
          Column 3: D Minor (ii)

        ...

       * Rows: Cycle through keys (C → G → D → A, etc.).
    
       * Toggle between key groups (C/G/D/A, E/B/F#/C#, etc.) with the Chord Cycle button.
    
       * Copy/Paste: Press Save to copy a row of chords to Favorites Mode.



Available Chord Types
        
        The code supports 12 chord types, each with a distinct color:
        
        Chord Type	Notes (Semitone Offsets)	Color
        Major	0, 4, 7	Green
        Minor	0, 3, 7	Blue
        Diminished	0, 3, 6	Red
        Augmented	0, 4, 8	Yellow
        SUS4	0, 5, 7	Purple
        ADD9	0, 4, 7, 14	Pink
        7th	0, 4, 7, 10	Magenta
        MAJ7	0, 4, 7, 11	Teal
        MIN7	0, 3, 7, 10	Lavender
        NINTH	0, 4, 7, 10, 14	Cyan
        MAJ9	0, 4, 7, 11, 14	Maroon
        MIN9	0, 3, 7, 10, 14	Lime


Utility Buttons
 
    Button 0 (Mode Toggle): Cycles through modes (Global → Favorites → Circle of Fifths).
    
   Button 8 (Save/Clear):
    
    In Global/Circle Mode: Enters save mode.
    
    In Favorites Mode: Toggles delete mode (hold to clear all).

   Button 16 (Octave Shift):
    
    In Global Mode: Cycles octave ranges.
    
    In Favorites Mode: Toggles octave shift for saved chords.
    
    In Circle Mode: Changes octave.
    
   Button 24 (Chord Cycle):
    
    In Global Mode: Cycles all rows to the next chord type.
    
    In Favorites Mode: Toggles strum direction (up/down).
    
    In Circle Mode: Cycles key groups (C/G/D/A → E/B/F#/C# → F/Bb/Eb/Ab).

Special Features
Strumming:

    Plays chord notes sequentially (up or down) with adjustable delay (STRUM_DELAY).
    
    Accelerometer Effects:
    
    Tilting the device adjusts pitch bend and modulation.
    
    MIDI Output:
    
    Sends notes over USB/UART MIDI on channel 0.

Summary
Global Chord Mode: Chromatic grid with configurable chord types per row.

Favorites Mode: Save/recall custom chords with strumming and octave shift.

Circle of Fifths Mode: Diatonic chord progressions in multiple keys.

12 Chord Types: From basic triads to extended jazz chords, each with unique colors.

This design allows for quick chord playing, music theory exploration, and performance flexibility. 🎹
