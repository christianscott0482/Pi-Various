#written by: Christian Auspland
#Reads temperature data from I2C port and query's into a database
import smbus
import time
import sqlite3

#initialize class objects
bus = smbus.SMBus(1)
db = sqlite3.connect('logger.db')
cursor = db.cursor()
starttime = time.time()

count = 0

#create new table called logger in the SQLite3 database called logger.db
cursor.execute('''
    CREATE TABLE logger (ID INT, temp INT, year INT, month TEXT, day INT, hour INT, minute INT, second INT)
''')
db.commit()

#i2c address that the temp sensor is wired to
ADDRESS = 0x48

#reads temp sensor and sends data to database forever
while True:
    #grabs a few values from the local clock as well as the temp sensor
    temp_value = bus.read_byte(ADDRESS)
    year = time.strftime("%Y")
    month = time.strftime("%B")
    day = time.strftime("%d")
    hour = time.strftime("%H")
    minute = time.strftime("%M")
    second = time.strftime("%S")

    #sends the values to the database
    cursor.execute('''INSERT INTO logger (ID, temp, year, month, day, hour, minute, second)
                      VALUES(?,?,?,?,?,?,?,?)''', (count, temp_value, year, month, day, hour, minute, second))
    db.commit()

    #prints a few value to the terminal for error checking
    print time.strftime("%c")
    print temp_value

    #if 24 hours worth of values are found, start deleting the oldest
    #information in the database
    if (count >= (24 * 60)):
        rem = ((60 * 24) + 1 - count)
        cursor.execute('''DELETE FROM logger WHERE ID = ?''', (rem))
        db.commit()
    count += 1
    print count

    #logic that tells the program to wait here until exactly 1 minute has
    #passed. Uses local clock for comparison.
    time.sleep(60.0 - ((time.time() - starttime) % 60.0))

db.close()
