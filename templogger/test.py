#Written by: Christian Auspland
#produces html for a webpage that prints a plot of a temperature logger
print "Content-type: text/html"
print
    #Gives title
print "<title>Temperature Plot</title>"
print "<html>"
print   "<body>"
            #centers, prints some text, and prints the most recent graph
print       "<p style=\"text-align:center;\">"
print           "<font color=\"black\">"
print               "Christian Auspland<br>"
print               "ECE 331 Project 2<br>"
print               "<img src=\"plot.png\" alt=\"temperaturevtimeplot\" style=\"width:550px;height:400px;\">"
print           "</font>"
print       "</p>"
print   "</body>"
print "</html>"
