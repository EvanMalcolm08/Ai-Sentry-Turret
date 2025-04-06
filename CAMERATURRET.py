import cv2
import numpy as np
import time
import serial

# Initialize serial connection to Arduino
arduino = serial.Serial('/dev/tty.usbserial-A10NADEJ', 115200)
time.sleep(2)  # Let connection settle

cap = cv2.VideoCapture(0)

# HSV color range for GREEN (wide)
lower_color = np.array([35, 50, 50])
upper_color = np.array([90, 255, 225])

# Smoothing weights
smooth_x = 0
smooth_y = 0
alpha = 0.5  # Smoothing factor (lower = more smoothing)

while True:
    ret, frame = cap.read()
    if not ret:
        break

    height, width, _ = frame.shape
    center_x, center_y = width // 2, height // 2

    # Convert to HSV and threshold
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv, lower_color, upper_color)

    # Find contours
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    if contours:
        largest = max(contours, key=cv2.contourArea)
        area = cv2.contourArea(largest)
        if area > 1000 and area < 5000:  # Upper bound added

            x, y, w, h = cv2.boundingRect(largest)
            object_x = x + w // 2
            object_y = y + h // 2

            # --- Apply smoothing ---
            smooth_x = int(alpha * object_x + (1 - alpha) * smooth_x)
            smooth_y = int(alpha * object_y + (1 - alpha) * smooth_y)

            # Draw bounding box and center
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
            cv2.circle(frame, (smooth_x, smooth_y), 5, (0, 0, 255), -1)

            # Calculate distances
            dx = smooth_x - center_x
            dy = smooth_y - center_y

            # Buffer zone
            buffer = 40
            cv2.rectangle(frame, (center_x - buffer, center_y - buffer),
                          (center_x + buffer, center_y + buffer), (255, 255, 0), 2)

            # Decide if centered
            if abs(dx) <= buffer and abs(dy) <= buffer:
                status = "Centered"
                color = (0, 255, 0)
            else:
                status = "Tracking"
                color = (0, 0, 255)
                message = f"X:{dx},Y:{dy}\n"
                print("Sending to Arduino:", message.strip())
                arduino.write(message.encode())

            # Status + DX DY on screen
            cv2.putText(frame, f'DX: {dx}, DY: {dy}', (10, 30),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)
            cv2.putText(frame, status, (10, 60),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, color, 2)

    # Show frame
    cv2.imshow('Turret Tracking', frame)

    # Quit with Q
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
arduino.close()
cv2.destroyAllWindows()
