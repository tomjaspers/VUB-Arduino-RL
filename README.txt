1) Install Arduino software.
2) Change the makefile to point to the arduino app. 
3) Possibly update FTDI drivers: http://www.ftdichip.com/Drivers/VCP.htm
3) maybe implement a getDetailedDirection


If you happen to 'break' Arduino and can't upload new programs:
	Remove the USB connection, close the IDE, open the IDE, open BareMinimum sketch, verify (compile), 
	hold the reset button, plugin the USB connection, click the upload button 1/2 a second after removing your finger from the 	reset button
	
Contact: cfscholl@vub.ac.be