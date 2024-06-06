The Pid Controller folder contain the arduino code for the pid controller. 
I tryed to make it readable has much as possible.
It use platformio so you can try to compile right away.
If not, look at the comments in platformio.ini

The Yogurt Fermenter folder contain the python code. It still messed up but working. You can try running yogurtdata.py after installing all libs.
It produce a real time graph of temperature and the power applyed from the pid controller.
You can send command to the controller and receive message from the controller.

The physical setup use an SSR relay controlled using a transistor to feed 5V to the SSR (10mA limited pin too).
