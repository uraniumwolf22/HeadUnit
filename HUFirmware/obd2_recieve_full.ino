#include <math.h>

int parse_obd2_full(char pid, char* data) {
  if (sizeof(*data) / sizeof(char)) == 0 { return 0; }

  switch (pid) {
    case 0x04: // Calculated engine load
      return data[0] / 2.55;
    case 0x05: // Engine coolant temperature
      return data[0] - 40;
    case 0x06: // Short term fuel trim Bank 1
    case 0x07: // Long term fuel trim Bank 1
    case 0x08: // Short term fuel trim Bank 2
    case 0x09: // Long term fuel trim Bank 2
      return (data[0]/1.28) - 100;
    case 0x0A: // Fuel pressure
      return 3 * data[0];
    case 0x0B: // Intake manifold absolute pressure
      return data[0];
    case 0x0C: // Engine speed
      return (256 * data[0] + data[1]) / 4;
    case 0x0D: // Vehicle speed
      return data[0];
    case 0x0E: // Timing advance
      return (data[0] / 2) - 64;
    case 0x0F: // Intake air temperature
      return data[0] - 40;
    case 0x10: // Air flow rate
      return (256 * data[0] + data[1]) / 100;
    case 0x11: // Throttle position
      return data[0] / 2.55;
    case 0x1F: // Run time since engine start
      return 256 * data[0] + data[1];
    case 0x21: // Distance traveled with MIL on
      return 256 * data[0] + data[1];
    case 0x22: // Fuel Rail Pressure
      return 0.079 * (256 * data[0] + data[1]);
    case 0x23: // Fuel Rail Gauge Pressure
      return 10 * (256 * data[0] + data[1]);
    case 0x24: // Oxygen sensors 1-8
    case 0x25: // Air-Fuel Equivalence Ratio
    case 0x26:
    case 0x27:
    case 0x28:
    case 0x29:
    case 0x2A:
    case 0x2B:
    case 0x34: // Oxygen sensors 1-8
    case 0x35: // Air-Fuel Equivalence Ratio
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3A:
    case 0x3B:
      return (2 / 65536) * (256 * data[0] + data[1]);
    case 0x2C: // Commanded EGR
      return (100 / 255) * data[0];
    case 0x2D: // EGR Error
      return (100 / 128) * data[0] - 100;
    case 0x2E: // Commanded evaporative purge
      return (100 / 255) * data[0];
    case 0x2F: // Fuel Tank Level Input
      return (100 / 255) * data[0];
    case 0x30: // Warm-ups since codes cleared
      return data[0];
    case 0x31: // Distance traveled since codes cleared
      return 256 * data[0] + data[1];
    case 0x32: // Evap. System Vapor Pressure
      return (256 * data[0] + data[1]) / 4;
    case 0x33: // Absolute Barometric Pressure
      return data[0];
    case 0x3C: // Catalyst Temperature B1S1
    case 0x3D: // Catalyst Temperature B2S1
    case 0x3E: // Catalyst Temperature B1S2
    case 0x3F: // Catalyst Temperature B2S1
      return (256 * data[0] + data[1]) / 10 - 40;
    case 0x42: // Control module voltage
      return (256 * data[0] + data[1]) / 1000;
    case 0x43: // Absolute load value
      return (100 / 255) * (256 * data[0] + data[1]);
    case 0x44: // Commanded Air-Fuel Equivalence Ratio
      return (2 / 65536) * (256 * data[0] + data[1]);
    case 0x45: // Relative throttle position
      return (100 / 255) * data[0];
    case 0x46: // Ambient air temperature
      return data[0] - 40;
    case 0x47: // Absolute throttle position B
    case 0x48: // Absolute throttle position C
    case 0x49: // Accelerator pedal position D
    case 0x4A: // Accelerator pedal position E
    case 0x4B: // Accelerator pedal position F
    case 0x4C: // Commanded throttle actuator
      return (100 / 255) * data[0];
    case 0x4D: // Time run with MIL on
    case 0x4E: // Time since trouble codes cleared
      return 256 * data[0] + data[1];
    case 0x50: // Maximum air flow rate
      return 10 * data[0];
    case 0x52: // Ethanol fuel percent
      return (100 / 255) * data[0];
    case 0x53: // Absolute Evap system Vapor Pressure
      return (256 * data[0] + data[1]) / 200;
    case 0x54: // Evap system vapor pressure
      return 256 * data[0] + data[1];
    case 0x55: // Short term secondary oxygen sensor trim Bank 1
    case 0x56: // Long term secondary oxygen sensor trim Bank 1
    case 0x57: // Short term secondary oxygen sensor trim Bank 2
    case 0x58: // Long term secondary oxygen sensor trim Bank 2
      return (100 / 128) * data[0] - 100;
    case 0x59: // Fuel rail absolute pressure
      return 10 * (256 * data[0] + data[1]);
    case 0x5A: // Relative accelerator pedal position
      return (100 / 255) * data[0];
    case 0x5B: // Hybrid battery pack remaining life
      return (100 / 255) * data[0];
    case 0x5C: // Engine oil temperature
      return data[0] - 40;
    case 0x5D: // Fuel injection timing
      return (256 * data[0] + data[1]) / 128 - 210;
    case 0x5E: // Engine fuel rate
      return (256 * data[0] + data[1]) / 20;
    case 0x61: // Commanded percent torque
    case 0x62: // Actual percent torque
      return data[0] - 125;
    case 0x63: // Engine reference torque
      return 256 * data[0] + data[1];
    case 0x66: // Mass air flow sensor
      return (256 * data[1] + data[2]) / 32;
    case 0x67: // Engine coolant temperature
    case 0x68: // Intake air temperature sensor
      return data[1] - 40;
    case 0x7C: // DPF temperature
      return (256 * data[0] + data[1]) / 10 - 40;
    case 0x8E: // Engine Friction - Percent Torque
      return data[0] - 125;
    case 0xA2: // Cylinder Fuel Rate
      return (256 * data[0] + data[1]) / 32;
    case 0xA4: // Transmission Actual Gear
      return (256 * data[2] + data[3]) / 1000;
    case 0xA5: // Commanded Diesel Exhaust Fluid Dosing
      return data[1] / 2;
    case 0xA6: // Odometer
      return (
        pow(2, 24) * data[0] +
        pow(2, 16) * data[1] +
        pow(2, 8) * data[2] +
        data[3]
      ) / 10;

      default: return 0;
  }

}
