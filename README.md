<img width="1280" height="640" alt="git (1)" src="https://github.com/user-attachments/assets/8920b256-2ba8-4988-b824-5351134eb4bd" />



# Mjölnir Worthiness Evaluation System


## Basic Details
### Team Name: Adithyan's Team


### Team Members
- Team Lead: Adithyan K - SCMS SCHOOL OF ENGINEERING AND TECHNOLOGY

### Project Description
An ESP8266 and MPU6050 powered Mjölnir that uses TinyML to analyze how you handle the hammer. It records your movements, classifies them, and determines whether you are worthy or not.

### The Problem (that doesn't exist)
People have spent centuries wondering who is worthy of wielding Mjölnir. Apparently, nobody thought to automate the decision. This project solves the completely unnecessary problem of determining whether someone is worthy of lifting Thor’s hammer using motion data and TinyML.

### The Solution (that nobody asked for)
We strapped an MPU6050 to Mjölnir, connected it to an ESP8266, and taught it to judge human movement using TinyML. The hammer watches how you lift, swing, shake, or simply stand there, then delivers the verdict that absolutely nobody asked for.

## Technical Details

### Technologies/Components Used

- ESP8266: Main microcontroller and handles the web interface
- MPU6050: 6-axis accelerometer and gyroscope for capturing motion
- TinyML: Logistic Regression model for motion classification
- Arduino IDE: Firmware development and uploading
- C/C++: Embedded firmware
- HTML, CSS & JavaScript: web interface


### Project Documentation

# Schematic & Circuit
<img width="1536" height="1024" alt="ChatGPT Image Sep 6, 2026, 04_33_35 AM" src="https://github.com/user-attachments/assets/cfdd97db-c727-446e-995d-cf95ce5d38f4" />

The MPU6050 is connected to the ESP8266 using I2C. VCC is connected to 3.3V, GND to GND, SDA to D2 (GPIO4), and SCL to D1 (GPIO5). The remaining MPU6050 pins are left unconnected.

<img width="1536" height="1024" alt="image" src="https://github.com/user-attachments/assets/f519a974-67ae-47cd-bbf3-50d6b2eef948" />

The schematic shows the ESP8266 and MPU6050 connected through the I²C interface, with D2 (GPIO4) used for SDA and D1 (GPIO5) used for SCL. Both modules share a common 3.3V supply and ground, while the unused MPU6050 pins are left unconnected.

# Build Photos

<img width="1600" height="900" alt="WhatsApp Image 2026-09-06 at 4 31 14 AM (2)" src="https://github.com/user-attachments/assets/6097c886-30eb-4853-adf8-200ee8b42659" />

The image shows the inner part of the hammer head, consisting of an ESP8266 and an MPU6050

<img width="900" height="1600" alt="WhatsApp Image 2026-09-06 at 4 31 14 AM" src="https://github.com/user-attachments/assets/48351b54-d249-4ace-91b8-4e117b924cc7" />

- Connect the MPU6050 to the ESP8266 using I²C, with SDA connected to D2 (GPIO4) and SCL connected to D1 (GPIO5).
- Set up the ESP8266 firmware to read acceleration and gyroscope data from the MPU6050.
- Collect motion data for different actions such as IDLE, LIFT, SWING, and SHAKE.
- Train the TinyML model using the collected motion data and extract the most useful motion features.
- Deploy the trained model to the ESP8266 for on-device motion classification.
- Build the web interface to display the motion, confidence, worthiness score, and final verdict.
- Connect the web interface to the ESP8266, allowing the user to start a worthiness trial directly from the browser.
- Run the trial by collecting a short motion window, classifying it, and displaying the final WORTHY or NOT WORTHY verdict.

<img width="900" height="1600" alt="WhatsApp Image 2026-09-06 at 4 31 14 AM (1)" src="https://github.com/user-attachments/assets/d4b17645-0a96-4634-bc5a-3d26a0e8f6ae" />

The final build consists of an ESP8266 and MPU6050 working together to evaluate the user's motion. The MPU6050 captures acceleration and gyroscope data, while the ESP8266 processes the data using the TinyML model and determines the type of movement.

The ESP8266 also hosts a web interface where the user can start the worthiness trial and view the motion, confidence, score, and final WORTHY or NOT WORTHY verdict.

<img width="722" height="1600" alt="image" src="https://github.com/user-attachments/assets/228e0301-191f-4488-be3c-f7f2b533859b" />

<img width="722" height="1600" alt="image" src="https://github.com/user-attachments/assets/052dcada-ec70-46ba-a07c-99f26648b822" />

The image consist of the web interface.

### Project Demo

# Video
[https://drive.google.com/file/d/1dUhmisIHGxUUYNEgrb0NRKkWY83q-mWO/view?usp=drivesdk]
Th video demonstrates the components, the working and also a working of the project.

## Team Contributions
- Adithyan K -I've done all the work by myself.

---
Made with ❤️ at TinkerHub Useless Projects 

![Static Badge](https://img.shields.io/badge/TinkerHub-24?color=%23000000&link=https%3A%2F%2Fwww.tinkerhub.org%2F)
![Static Badge](https://img.shields.io/badge/UselessProjects--26-26?link=https%3A%2F%2Ftinkerhub.org%2Fevents%2F1M8ORET9A1%2Fuseless-projects-3.0)



