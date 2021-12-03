#!/bin/bash
PYTHONPATH=/home/pi/.local/lib/python3.9/site-packages python3 /home/pi/SGC/sql.py &
PYTHONPATH=/home/pi/.local/lib/python3.9/site-packages python3 /home/pi/SGC/camera.py &
