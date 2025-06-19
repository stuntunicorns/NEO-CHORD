The NEO-CHORD is a USB-MIDI controller that plays full chords at the push of a button.  Written for the Adafruit NeoTrellisM4. 

Modes Overview
 
   Mode 1: Global Chords Mode (Chromatic Grid)
   
    * Chord Layout: 4 rows × 6 columns. 1st and Last columns are utility.
    
    * Each row's chord type can be changed (e.g., Major, Minor, 7th) using utility buttons
    
    Example: Major Chords Grid
    
          Col 1 - Col 2 - Col 3 - Col 4 - Col 5 - Col 6  
    Row 1   C       C#      D       D#      E       F    Octave 3 
    Row 2   F#      G       G#      A       A#      B    Octave 3   
    Row 3   C       C#      D       D#      E       F    Octave 4
    Row 4   F#      G       G#      A       A#      B    Octave 4
    
    * Columns represent semitone steps (chromatic scale).
    
    * Chord Selection: Change Chord Grid using button 24
    
    * Rows: Each row chord type can be changed independent of other rows in the grid (configurable via "row type" buttons on the right).
    
    * Octave Shifting: Toggle between octave ranges (1-2, 3-4, 5-6) using the button 16.

  Mode 2: Favorites Mode
  
         * Layout: 4 rows × 7 columns (28 slots total) to save/recall custom chords.
         
         * Saved chords are highlighted in their chord-type color.
 
    Features:

         * Save Chords:
            In Global Chord Mode or Circle of Fifths Mode, press Save (button 8) and press any chord button. The mode changes to "Favorites Mode".
            Press an empty slot (Row if saving from "Circle of Fifths" Mode) in Favorites Mode to save.
         
         * Delete Chords:
            Press Save to toggle delete mode (button 8) , then press a saved chord. To clear the entire grid hold button 8 for 3 seconds. 
      
         * Strumming: Toggle strum mode using button (button 24). Strumming (up/down) toggled by button 16
      
         * Octave Shift: When not in strum mode, button 16 shifts saved chords ±1 octave relative to the current octave of the chord.

  Mode 3: Circle of Fifths Mode
    
    Layout: 7 columns (one for each diatonic chord in a key) 4 rows and 3 grids (for different keys in the circle of fifths).

     * Chord Progressions:
     
          Columns: Represent chords in a key (I, IV, V, ii, iii, vi, vii°).
  
          Example: Grid 1, Row 1 = Chords in the key of C
                I        IV       V        ii      iii       vi       vii°
              Col 1    Col 2    Col 3    Col 4    Col 5    Col 6     Col 7
    Row 1 -  C Major  F Major  G Major  D Minor  E Minor  A Minor  B Diminished
  

       * Rows: Cycle through keys (C → G → D → A, etc.).
    
       * Toggle between key group grids (C/G/D/A, E/B/F#/Db, and Ab/Eb/Bb/F) with the Chord Cycle button (button 24).
    
       * Copy/Paste: Press Save to copy a row of chords to Favorites Mode (button 8).

       * Octave Shifting: Toggle between octave ranges using the button 16.



Available Chord Types
        
        The code supports 12 chord types, each with a distinct color:
        
        Chord Type	               Notes (Semitone Offsets)            	Color
        
        Major                             0, 4, 7                      Green
        Minor                             0, 3, 7                      Blue
        Diminished                        0, 3, 6                      Red
        Augmented                         0, 4, 8                      Yellow
        SUS4                              0, 5, 7                      Peach
        ADD9                              0, 4, 7, 14                  Pink
        7th                               0, 4, 7, 10                  Orange
        MAJ7                              0, 4, 7, 11                  Lime
        MIN7                              0, 3, 7, 10                  Teal
        NINTH                             0, 4, 7, 10, 14              Purple
        MAJ9                              0, 4, 7, 11, 14              Light Blue
        MIN9                              0, 3, 7, 10, 14              Lavender


Utility Buttons
 
    Button 0 (Mode Toggle): Cycles through modes (Global → Favorites → Circle of Fifths).
    
   Button 8 (Save/Clear):
    
    In Global/Circle Mode: Enters save mode.
    
    In Favorites Mode: Toggles delete mode (hold to clear all).

   Button 16 (Octave Shift):
    
    In Global Mode: Cycles octave ranges.
    
    In Favorites Mode: Toggles octave shift for saved chords. Toggles strum direction (up/down) after entering strum mode (button 24) 
    
    In Circle Mode: Changes octave.
    
   Button 24 (Chord Cycle):
    
    In Global Mode: Cycles all rows to the next chord type.
    
    In Favorites Mode: Toggles strum mode on/off.
    
    In Circle Mode: Cycles key groups (C/G/D/A → E/B/F#/Db → Ab/Eb/Bb/F). 

Special Features
Strumming, Pitch Bend and Modulation:

    Plays chord notes sequentially (up or down) with adjustable delay (STRUM_DELAY).
    
    Accelerometer Effects:
    
    Tilting the device adjusts pitch bend and modulation.
    
    MIDI Output:
    
    Sends notes over USB/UART MIDI on channel 0.

Summary
Global Chord Mode: Chromatic grid with configurable chord types. By grid or row.

Favorites Mode: Save and play chords with strumming and octave shift.

Circle of Fifths Mode: Diatonic chord progressions in 12 keys.

12 Chord Types: From basic triads to extended jazz chords, each with unique colors.

This design allows for quick chord playing, music theory exploration, and performance flexibility. 🎹
