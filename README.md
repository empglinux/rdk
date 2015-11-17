# RDK - Referenced Development Kit

 * Copyright (c) 2009-2015, Intel Corporation.
 
## Overview

RDK is an open source framework, it includes two parts.
* User space demon application, it takes actions according to the data that get from its driver.
* Hardware drivers, it offers data to user space application and manages hardware.
     
 Support Intel Education Platforms:     
 * Golden Creek         
 Support Linux OS:       
 * Debian8.1     

## Dependencies

 * git	(tool for download RDK code)	    
 

## How to compile

  Compile needs root privilege:   
	$su root          

* Compile drivers under driver directory (rdk-drivers) :       
	$make   		   
         
* Compile sensord under sensord directory (sensord):      
	$make         
	$make install
	$make deb	( optional: will generate deb install package. )	    

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
