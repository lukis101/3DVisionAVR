# 3DVisionAVR
Partial NVIDIA 3D Vision Emitter emulation using AVR microcontrollers  

Implementation is solely based on driver behavior analysis and information available online without even touching one.  

Built using [LUFA library](https://github.com/abcminiuser/lufa) for handling USB communication.  
Atmel Studio 7 project files included, but binaries are built via makefile to keep references to LUFA minimal.  

Configured and tested on "Arduino Pro Micro" clone (Atmega32U4)  
but should be compatible or easy to port to other AVRs with native USB.  

The emitter logic uses single 16-bit timer and can be easily integrated into other projects.  
Flexible protocol description can support most currently known protocols. *Frame duty cycle is currently hardcoded!*

### Available operation modes:  
* **Free-run**: simple unsynchronized software flipping, good for compatibility or on-the-go testing.  
* **Hardware**: sync to external [VESA stereoscopic sync signal](http://3dvision-blog.com/forum/viewtopic.php?f=8&t=736).  
* **Driver**: flip directly from external function calls. No reference timer used like in original so there's a lot of jitter.  
* **Combined**: obtain frame polarity from driver but frames timed to hardware signal.  

## Notice  
This was developed for experimental purposes and is not in any way intended to be a replacement for the original product.