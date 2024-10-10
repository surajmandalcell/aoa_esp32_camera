import urllib.request
import cv2
import numpy as np

url = "http://192.168.2.62/cam-lo.jpg"

def avg_circles(circles, b):
    avg_x, avg_y, avg_r = np.mean(circles[0, :, :], axis=0)
    return int(avg_x), int(avg_y), int(avg_r)


def take_measure(threshold_img, threshold_ln, minLineLength, maxLineGap):
    imgResp = urllib.request.urlopen(url)
    imgNp = np.array(bytearray(imgResp.read()), dtype=np.uint8)
    img = cv2.imdecode(imgNp, -1)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    # Detect circles (outer rim of the gauge)
    circles = cv2.HoughCircles(
        gray, cv2.HOUGH_GRADIENT, 1, 20, param1=50, param2=30, minRadius=0, maxRadius=0
    )

    if circles is not None:
        x, y, r = avg_circles(circles, len(circles[0]))

        # Draw center and circle
        cv2.circle(img, (x, y), r, (0, 255, 0), 3, cv2.LINE_AA)
        cv2.circle(img, (x, y), 2, (0, 255, 0), 3, cv2.LINE_AA)

        # Create mask for bottom right 2/6th
        mask = np.zeros(gray.shape, np.uint8)
        cv2.ellipse(mask, (x, y), (r, r), 0, 150, 330, 255, -1)

        # Apply mask to original image
        masked_img = cv2.bitwise_and(img, img, mask=mask)

        # Convert to HSV color space
        hsv = cv2.cvtColor(masked_img, cv2.COLOR_BGR2HSV)

        # Define range for white color
        lower_white = np.array([0, 0, 200])
        upper_white = np.array([180, 30, 255])

        # Create a binary image with where white areas are 255 and elsewhere are 0
        white_mask = cv2.inRange(hsv, lower_white, upper_white)

        # Find contours in the binary image
        contours, _ = cv2.findContours(
            white_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
        )

        if contours:
            # Find the largest contour (assuming it's the needle)
            largest_contour = max(contours, key=cv2.contourArea)

            # Get the moments of the largest contour
            M = cv2.moments(largest_contour)

            if M["m00"] != 0:
                # Calculate the centroid of the largest contour
                cx = int(M["m10"] / M["m00"])
                cy = int(M["m01"] / M["m00"])

                # Draw a line from the center of the gauge to the centroid of the needle
                cv2.line(img, (x, y), (cx, cy), (0, 0, 255), 2)

                # Calculate angle
                angle = np.arctan2(cy - y, cx - x)
                needle_angle = (np.degrees(angle) - 150) % 180

                # Map the angle to a reading (adjust these values based on your gauge)
                min_speed = 0
                max_speed = 160  # Assuming max speed of 160 knots for this section
                speed = min_speed + (needle_angle / 180) * (max_speed - min_speed)

                print(f"Speed: {speed:.1f} knots")

                cv2.putText(
                    img,
                    f"Speed: {speed:.1f} knots",
                    (50, 50),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    1,
                    (0, 255, 0),
                    2,
                    cv2.LINE_AA,
                )
            else:
                cv2.putText(
                    img,
                    "Can't determine needle position!",
                    (50, 50),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    1,
                    (0, 0, 255),
                    2,
                    cv2.LINE_AA,
                )
        else:
            cv2.putText(
                img,
                "Can't find the needle!",
                (50, 50),
                cv2.FONT_HERSHEY_SIMPLEX,
                1,
                (0, 0, 255),
                2,
                cv2.LINE_AA,
            )
    else:
        cv2.putText(
            img,
            "Can't see the gauge!",
            (50, 50),
            cv2.FONT_HERSHEY_SIMPLEX,
            1,
            (0, 0, 255),
            2,
            cv2.LINE_AA,
        )

    return img


while True:
    threshold_img = 200
    threshold_ln = 50
    minLineLength = 40
    maxLineGap = 10

    img = take_measure(threshold_img, threshold_ln, minLineLength, maxLineGap)
    cv2.imshow("Winter-7423 Airspeed Indicator Reader", img)

    if ord("q") == cv2.waitKey(10):
        break

cv2.destroyAllWindows()
