#!/usr/bin/env python3
import pygame
import pygame.camera
import requests
from datetime import datetime
import time
import os
import sys

#Setting for WebCam
#Captured image dimensions. It should be less than or equal to the maximum dimensions acceptable by the camera
width = 640
height = 480
#initializing PyGame and Camera
pygame.camera.init()
pygame.init()

#Server address
url_server = "http://34.101.153.146"


#File Directories to save images
dir_camera_back = '/home/pi/SGC/Camera/camera_back/'
dir_camera_side = '/home/pi/SGC/Camera/camera_side/'
dir_camera_top = '/home/pi/SGC/Camera/camera_top/'

#Specifying the camera to be used for capturing images
camlist = pygame.camera.list_cameras()
if camlist:
    cam_top = pygame.camera.Camera('/dev/video0',(width,height))
    cam_side = pygame.camera.Camera('/dev/video2',(width,height))
    cam_back = pygame.camera.Camera('/dev/video4',(width, height))
#Preparing a resizablewindow of the spesified size for displaying the image
#window = pygame.display.set_mode((width,height),pygame.RESIZABLE)

#Saving the captured image
def takeImage() :
    try :
        cam_top.start()
        image = cam_top.get_image()
        cam_top.stop()
        #window.blit(image,(0,0))
        #pygame.display.update()
        pygame.image.save(image,dir_camera_top +'Top_{}.png'.format(timestamp))
    except :
        print('cannot start cam_top, please check camera connection')
        
    try:   
        cam_back.start()
        image = cam_back.get_image()
        cam_back.stop()
        #window.blit(image,(0,0))
        #pygame.display.update()
        pygame.image.save(image,dir_camera_back + 'Back_{}.png'.format(timestamp))
    except :
        print('cannot start cam_back, please check camera connection')
        
    try:
        cam_side.start()
        image = cam_side.get_image()
        cam_side.stop()
        #window.blit(image,(0,0))
        #pygame.display.update()
        pygame.image.save(image,dir_camera_side + 'Side_{}.png'.format(timestamp))
    except :
        print('cannot start cam_side, please check camera connection')
               
#Sending images to server
def sendImageFile_top(): 
    #Sending data to server
    try :
        url_top = '{}/SGC/post_camera_top.php'.format(url_server)
        file_top = {'top_camera': open(dir_camera_top + 'Top_{}.png'.format(timestamp), 'rb')}
        r = requests.post(url_top, files=file_top) 
        print("top : {}".format(r))
        return r
    except :
        pass

def sendImageFile_back():
    try :
        url_back = '{}/SGC/post_camera_back.php'.format(url_server)
        file_back = {'back_camera': open(dir_camera_back + 'Back_{}.png'.format(timestamp), 'rb')} 
        r= requests.post(url_back, files=file_back)
        print("back : {}".format(r))
        return r
    except :
        pass
        
def sendImageFile_side():
    try : 
        url_side = '{}/SGC/post_camera_side.php'.format(url_server)
        file_side = {'side_camera': open(dir_camera_side + 'Side_{}.png'.format(timestamp), 'rb')} 
        r = requests.post(url_side, files=file_side)
        print("side : {}".format(r))
        return r
    except :
        pass

#Send image file names to sql database
def sendImageName(flag_top,flag_back, flag_side):
    if str(flag_top) == '<Response [200]>' :
        filename_top = 'Top_{}.png'.format(timestamp)
    else :
        filename_top = ""
    if str(flag_back) == '<Response [200]>' :
        filename_back = 'Back_{}.png'.format(timestamp)
    else:
        filename_back = ""
    if str(flag_side) == '<Response [200]>' :
        filename_side = 'Side_{}.png'.format(timestamp)
    else :
        filename_side = ""
    try :
        filenames = {"top_camera_filename" : filename_top, "back_camera_filename" : filename_back, "side_camera_filename" : filename_side} 
        flag_sql = requests.post("{}/SGC/post_image_name.php".format(url_server), data=filenames)
        return flag_sql
    except :
        pass

#Delete recent image file
def deleteFiles() :
    if os.path.exists(dir_camera_side + 'Side_{}.png'.format(timestamp)):
        os.remove(dir_camera_side + 'Side_{}.png'.format(timestamp))
    else:
        print("The file on camera_side directory does not exist")
        
    if os.path.exists(dir_camera_top + 'Top_{}.png'.format(timestamp)):
        os.remove(dir_camera_top + 'Top_{}.png'.format(timestamp))
    else:
        print("The file on camera_top directory does not exist")
        
    if os.path.exists(dir_camera_back + 'Back_{}.png'.format(timestamp)):
        os.remove(dir_camera_back + 'Back_{}.png'.format(timestamp))
    else:
        print("The file on camera_back directory does not exist")
        
    
#Main
running = True
if __name__ == '__main__':
    while running == True:
        
        #for event in pygame.event.get():
           #if event.type == pygame.QUIT:
               #running = False
               
        timestamp = datetime.now().strftime("%y%m%d_%H%M%S")
        
        takeImage()
        time.sleep(1)
        
        flag_top = sendImageFile_top()
        time.sleep(1)
        flag_back = sendImageFile_back()
        time.sleep(1)
        flag_side = sendImageFile_side()
        time.sleep(1)
        
        #Insert filename into sql database
        flag_sql = sendImageName(flag_top,flag_back, flag_side)
        print("sql : {}".format(flag_sql))
        
        #Delete recent file from computer
        deleteFiles()
        
        time.sleep(300)
        
    #pygame.quit()
    #sys.exit(0)
            
