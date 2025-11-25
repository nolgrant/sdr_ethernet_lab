#! /usr/bin/python3
import cgi, cgitb
import sys
import subprocess
import ctypes
from ctypes import c_int, POINTER, c_char_p

# Create instance of FieldStorage
form = cgi.FieldStorage()

# Get data from fields
adc_freq_hz  = int(form.getvalue('adc_freq_hz'))
tune_freq_hz  = int(form.getvalue('tune_freq_hz'))
streaming = form.getvalue('streaming')

# Send the result to the browser
print ("Content-type:text/html")
print()
print ("<html>")
print ('<head>')
print ("<title>Radio Configurator</title>")
print ('</head>')
print ('<body>')
print ("<h2>Radio Configurator</h2>")
print ("Setting up the radio now...")
print ("ADC Freq = %d, Tune Freq = %d" %(adc_freq_hz,tune_freq_hz))
if (streaming == "streaming"):
  streambool = True
  print ("streaming is Enabled<br>")
else :
  print ("streaming is Disabled<br>")
  streambool = False

# Load the shared library
# The extension will vary by OS (.so, .dll, .dylib)
# Be aware that calling the actual 'main' function this way might not be portable
# across all compilers/OSes, a better practice is to have a separate function
# that main() then calls.
# print ("about to import streamIQ<br>")
lib = ctypes.CDLL('./tuneRadio.so')
# print ("imported<br>")
# Define the function's return type and argument types
lib.main.restype = c_int
lib.main.argtypes = c_int, POINTER(c_char_p)

# print ("about to define<br>")

# Prepare arguments in C format
def make_args(cmd):
    # Encode string arguments to bytes and create a list
    args = cmd.encode().split()
    # Create a C-style array of char* pointers
    return (c_char_p * len(args))(*args)

# print ("defined<br>")

# Call the C main function with arguments
command_line = 'tuneRadio ' + str(adc_freq_hz) + ' ' + str(tune_freq_hz) + ' ' + str(streambool)
# 'sudo -S <<< student ./streamIQ '
# print ("test<br>")

print("Command line call: " + command_line + "<br>")
# print ("test2<br>")
c_args = make_args(command_line)
argc = len(c_args)
result_code = lib.main(argc, c_args)
print("C function returned:", result_code)
print ("done<br>")
print ('</body>')
print ('</html>')
