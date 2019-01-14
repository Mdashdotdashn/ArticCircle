// Categories:
// 0x01 = Modulator
// 0x02 = Sequencer
// 0x04 = Clocking
// 0x08 = Quantizer
// 0x10 = Utility
// 0x20 = MIDI
// 0x40 = Logic
// 0x80 = Other

#define HEMISPHERE_AVAILABLE_APPLETS 4

//////////////////  id  cat   class name
#define HEMISPHERE_APPLETS { \
    DECLARE_APPLET( 4, 0x08, DualQuant), \
    DECLARE_APPLET( 3, 0x02, GridsChannel), \
    DECLARE_APPLET( 1, 0x04, Metronome), \
    DECLARE_APPLET( 2, 0x40, ProbabilisticGate), \
}
/*    DECLARE_APPLET(127, 0x80, DIAGNOSTIC), \ */
