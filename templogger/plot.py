#Written by: Christian Auspland
#Gathers temperature and time data from SQLite database and creates a plot
import numpy
import matplotlib.pyplot as plt
import sqlite3
import time

#class object setup
db = sqlite3.connect('logger.db')
cursor = db.cursor()
starttime = time.time()

#endless loop of producing new plot every minute
while True:
    #initializes lists
    myList_temp = []
    myList_time = []
    #select which columns from SQLite database I want to view
    cursor.execute('''SELECT temp, year, month, day, hour, minute, second FROM logger''')
    
    #runs once for every entry in the SQLite table logger
    for row in cursor:
        #loads database values as a float, and throws them into a list
        dbtemp = [float(row[0])]
        year = [float(row[1])]
        day = [float(row[3])]
        second = [float(row[6])]

        #loads database values as floats for math
        hour = float(row[4])
        minute = float(row[5])
    
        #calculates what minute of the day that this temp was found
        timein = (hour * 60) + minute
        #loads that minute into list
        dbtime = [timein]

        #loads values into list to be plotted
        myList_temp = myList_temp + dbtemp
        myList_time = myList_time + dbtime

    #plot degress vs. time in red, with specific limits on both axes
    #set. Specific tick intervals hand written. Gridded. Labels and
    #a title added. Finally, saved to lighttpd python root file
    plt.plot(myList_time, myList_temp, 'r')
    plt.ylim(15, 40)
    plt.xlim(0, 1440)
    plt.xticks([0,180, 360, 540, 720, 900, 1080, 1260, 1440], ['12:00', '3:00', '6:00', '9:00', '12:00', '15:00', '18:00', '21:00', '24:00'], rotation = 'vertical')
    plt.grid(True)
    plt.xlabel('Time of day')
    plt.ylabel('Degrees (Celsius')
    plt.title('Temperature Log')
    plt.savefig('/var/www/html/cgi-bin/plot.png', bbox_inches='tight')

    #time logic that waits here until one minute has passed on the
    #computers local clock
    time.sleep(60.0 - ((time.time() - starttime) % 60.0))
