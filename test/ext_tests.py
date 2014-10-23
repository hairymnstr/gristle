#!/usr/bin/env python
import sys, os, subprocess
from PIL import Image, ImageDraw

force_create = False

if (not os.path.exists("testext.img")) or (force_create):
    
    # make the empty file for the filesystem
    pr = subprocess.Popen(["dd", "if=/dev/zero", "of=testext.img", "bs=1M", "count=512"])
    pr.wait()

    # format the file system
    pr = subprocess.Popen(["mke2fs", "testext.img"], stdin=subprocess.PIPE)
    pr.stdin.write("y\n")
    pr.wait()

if not os.path.isdir("temp"):
    os.mkdir("temp")

print "Mount image..."    
pr = subprocess.Popen(["sudo", "mount", "-o", "loop", "testext.img", "temp"])
pr.wait()

if not os.path.isdir("temp/static"):
    print "Creating directory on new volume"
    pr = subprocess.Popen(["sudo", "mkdir", "temp/static"])
    pr.wait()
    print "Making the directory writeable"
    pr = subprocess.Popen(["sudo", "chmod", "0777", "temp/static"])
    pr.wait()

if not os.path.exists("temp/static/testimg.png"):
    print "Creating a test image"
    image = Image.new('RGBA', (640, 480))
    draw = ImageDraw.Draw(image)
    draw.ellipse((120, 40, 520, 440), fill = 'blue', outline = 'blue')
    print "Saving to test file system"
    image.save("temp/static/test_image.png")

if not os.path.exists("temp/logs"):
    print "Creating log folder"
    pr = subprocess.Popen(["sudo", "mkdir", "temp/logs"])
    pr.wait()
    
    print "Making the directory writeable"
    pr = subprocess.Popen(["sudo", "chmod", "0777", "temp/logs"])
    pr.wait()

if not os.path.exists("temp/logs/test.txt"):
    print "Creating test text file"
    fw = open("temp/logs/test.txt", "w")
    fw.write("Hello world\n")
    fw.close()
    
print "Unmount image..."
pr = subprocess.Popen(["sudo", "umount", "temp"])
pr.wait()