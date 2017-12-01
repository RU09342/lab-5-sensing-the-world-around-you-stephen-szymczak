# Visualizing Data
In the next lab, we will start looking into how to take action on the information your microprocessor is receiving, for the last part of this lab, we will focus on visualizing the data. We will focus on three main methods of visualization. For each of these methods, you need to pick between 1-3 processors based on the part. As with Milestone 1, you will need to talk in this README about why you picked the processors you did (for one part, it is going to be painfully obvious). Overall, you should aim to use all five processors by the end of this part of the lab, however _YOU DO NOT NEED TO USE ALL FIVE FOR EACH PART_.

## RGB LED
Hopefully by this point you are fairly comfortable with how to control an RGB LED. To ease into this lab, you should take your sensors and have them correlate to the color, brightness, blinking speed, etc. of an RGB LED. If for your resistive sensor you picked a thermistor, it might make sense to have the RGB change from a "Blue" color to a "Red" color as it heats up. For this, you should pick 3 processors to perform this on. You do not need to use all three sensors on each of the boards. Pick one sensor per board and work off of that.

## LCD Display
Now that you are getting sensor data and acting on it, why don't you actually try to display the information the user in actual numbers. Using the MSP430FR6989, convert the information from all three of your sensors to a human readable value on the on-board LCD. Fair warning, *DO NOT TRY TO REINVENT THE WHEEL*. Make sure you give the resource explorer a good looking through to see what TI is going to provide you.

## PC Visualization
While UART is awesome for talking between processors, it also can be used to stream data back to your computer which you can use software to read. If you want to be adventurous (or if you have experiences in other languages like Python or Java) you can try to make your own visualization software for looking at the sensor data. Instead of that, you need to be able to collect and visualize data through MATLAB using the "Serial Toolbox". It is up to you to figure out how fancy you want your plots to be, but at a minimum, your processor needs to be able to communicate back to MATLAB and you need to be able to plot the data over time back on your laptop. Since this is all UART, I would expect you to do this for at least two processors.

## Now its your turn
For the finale of this lab, pick a processor and run at least two of these visualizations at the same time. You also should look at using multiple channels on the internal ADC, although this is not required.


## Deliverables
For this part of the lab, you need to be able to organize your submissions based on the part of the lab it is fulfilling. If this means using a ton of folders, be my guest, but at the end of the day, I am going to be grading these as if I am someone coming to your repository for information. This whole part can be summarized in one large README which should be *HEAVILY* focused on how to actually implement and use your code.
fff
