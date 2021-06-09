// PT100 Wire Configurations
// Modify 'MAX31865_4WIRE' to suit number of wires of PT100. Ensure correct jumpers are used on PCB
// OPTIONS:
// MAX31865_4WIRE
// MAX31865_3WIRE
// MAX31865_2WIRE
#define CH1_WIRE_CONFIG     MAX31865_4WIRE    // Temperautre channel 1
#define CH2_WIRE_CONFIG     MAX31865_4WIRE    // Temperautre channel 2
#define CH3_WIRE_CONFIG     MAX31865_4WIRE    // Temperautre channel 3
#define CH4_WIRE_CONFIG     MAX31865_4WIRE    // Temperautre channel 4
#define CH5_WIRE_CONFIG     MAX31865_4WIRE    // Temperautre channel 5
#define CH6_WIRE_CONFIG     MAX31865_4WIRE    // Temperautre channel 6
#define CH7_WIRE_CONFIG     MAX31865_4WIRE    // Temperautre channel 7
#define CH8_WIRE_CONFIG     MAX31865_4WIRE    // Temperautre channel 8
#define CH9_WIRE_CONFIG     MAX31865_4WIRE    // Temperautre channel 9
#define CH10_WIRE_CONFIG    MAX31865_4WIRE    // Temperautre channel 10



// Test length in seconds (test can be ended prematurely using button)
#define testLength          604800


// Sampling frequency in Hz - 10 Hz is fastest sample speed
#define samplingFrequency   10


// Unit of temperature 'C' or 'K' - if unit is not set correctly, program will default to C
#define unit  'K'
