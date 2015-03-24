# RDK - Referenced Development Kit

 * Copyright (c) 2009-2014, Intel Corporation.
 
## Overview

RDK is an open source framework, it includes two parts.
* User space demon application, it takes actions according to the data that get from its driver.
* Hardware drivers, it offers data to user space application and manages hardware.
     
 Support Intel Education Platforms:     
 * Stone Point, Buck Point, Beach Point, Match Point, Marble Point         
 Support Linux OS:       
 * Debian7, Ubuntu     

## Dependencies

 * git	(tool for download RDK code)
 * libgtk2.0-dev  (gtk+ develop library)
 * libxtst-dev	(x11 testing library)
 * libdbus-1-dev	(dbus library)
 * libdbus-glib-1-dev	(dbus-glib library)

## How to compile

  Compile needs root privilege:   
	$su root          

* Compile driver under driver directory:    
	$make      

* Compile functionkeys under functionkeys directory:      
	$./autogen.sh     
	$make install     

* Compile sensord under sensord directory:      
	$make         
	$make install        

Finally :    
	$reboot     
 
## License

 * User space application program is free software; you can redistribute it and/or modify it
   under the terms and conditions of the GNU Lesser General Public License,
   version 2.1, as published by the Free Software Foundation.

   This program is distributed in the hope it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
   more details.
   
 * Driver program is free software; you can redistribute it and/or modify it
   under the terms and conditions of the GNU General Public License,
   version 2, as published by the Free Software Foundation.
 
   This program is distributed in the hope it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.  
