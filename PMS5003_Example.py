# A class to interface with the i2C PMS5003 Sensor
#   Written by Team CCC
#

## [imports]
from smbus2 import SMBus    # Used to communicate with the i2c bus on pi

# Main Class
#   Functions:
#     Constructor   - Sets up up the PMS5003 Sensor
#     getData       - Returns the CPM
class PMS5003_Sensor:

    # Constructor class, sets drone up
    #   Takes in, the Address the PMS5003 Sensor is at
    def __init__(self, ADDRESS):

        ## [Default Values] (centred around Loughborough)
        
        # Initialising the I2C Bus
        self.BUS = SMBus(1)

        # Address of the i2c device
        self.ADDRESS = ADDRESS

        # The Number of Blocks of 8 bits to read from the device
        self.BLOCKSIZE = 2   # [WARNING] If set incorrectly the i2c bus will block out

        # Default value of the PMS5003 Sensor
        self.combine = 0

        return

    # Function to get data from the I2C device
    #   Takes in a registar value that you want to retrive data from.
    #       REGISTAR -> 1 = PM1, 2 = PM2.5, 3 = PM10
    def getData(self, REGISTAR:int):
        # Attempt read from bus
        try:
            # Attempt read from device
            dat = self.BUS.read_i2c_block_data(self.ADDRESS, REGISTAR, self.BLOCKSIZE)

            # Combine the Values
            self.combine = dat[1] << 8 | dat[0]

            # Retrun the Value
            return self.combine, 0
        # Returns 1 on failure
        except:
            return self.combine, 1

'''
# Testing
PMS5003 = PMS5003_Sensor(0x2d)

print(PMS5003.getData(1))
'''