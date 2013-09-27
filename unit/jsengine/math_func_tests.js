/*
 *  PACrunner JavaScript Engine
 *
 *  Copyright (C) 2013  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms and conditions of the GNU Lesser General Public
 *  License version 2.1 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

var a;
a = Math.cos(1);
alert("cos(1) = .54\n");
alert(a);
a = Math.abs(-1);
if (a == 1) {
	alert("abs test passed\n");
} else {
	alert("abs test failed\n");
}

a = Math.acos(.54);
alert("acos(.54) = 1\n");
alert(a);

a = Math.asin(.84);
alert("asin(.84) = 1\n");
alert(a);

a = Math.atan(1.55);
alert("atan(1.55) = 1\n");
alert(a);

a = Math.atan2(1, 1);
alert("atan2(1 ,1) = 1\n");
alert(a);

a = Math.ceil(5.5);
if (a == 6) {
	alert("ceil test passed\n");
} else {
	alert("ceil test failed\n");
	alert(a);
}

a = Math.floor(5.5);
if (a == 5) {
	alert("floor test passed\n");
} else {
	alert("floor test failed\n");
	alert(a);
}

a = Math.log(10);
alert("ln(10) = 2.3");
alert(a);

a = Math.pow(2 ,2);
if (a == 4) {
	alert("pow test passed\n");
} else {
	alert("pow test failed\n");
}

a = Math.round(5.7);
if (a == 6) {
	alert("round test passed\n");
} else {
	alert("round test failed\n");
}

a = Math.sqrt(4);
if (a == 2) {
	alert("sqrt test passed\n");
} else {
	alert("sqrt test failed\n");
}

a = Math.tan(1);
alert("tan(1) = 1.56");
alert(a);

a = Math.max( 1 , 2 , 5 , 6);
if (a == 6) {
	alert("max test passed\n");
} else {
	alert("max test failed\n");
}

a = Math.min( 1 , 2 , 5 , 6);
if (a == 1) {
	alert("min test passed\n");
} else {
	alert("min test failed\n");
}

alert("test done evaluating\n");

return 0;
