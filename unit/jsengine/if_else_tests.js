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

alert("*Check: If statements");
if (!0 == 0) {
	return 1;
}

if (!(3 == 5)) {
	alert("!(3 == 5)");
}

if (4 < 8) {
	alert("4 < 8");
}

if (10 > 9) {
	alert("10 > 9");
}

if (45 >= 45) {
	alert("45 >= 45");
}

if (34 <= 100) {
	alert("34 <= 100");
}

if (1 == (3 & 2)) {
	alert("1 == (3 & 2)");
}

if ("tree" == "tree") {
	alert("tree == tree");
}
if (4 != 3) {
	alert("4 != 3");
}

if ((79 == 3) || (4 == 4)) {
	alert("Matched 2nd OR condition");
}
if (5 < 10 && "test" == "test" && 5 > 2) {
	alert("45 < 10 && test == test");
}


alert("*Check: Elseif statement");
if ("seven" == "four") {
	alert("Failed: seven != four");
} else {
	alert("seven != four");
}

var a = 5;

if (a == 5) {
	alert("if test passed");
}

if (a == 5) {
	a = 9;
} else {
	alert("if else test failed");
}


if (a == 6) {
	alert("if else test failed");
} else {
	alert("if else test passed");
	a = 5;
}

if (a == 6) {
	alert("else if test failed");
} else if ( a == 5) {
	a = 7;
} else {
	alert("else if test failed");
}


if (a == 8) {
	alert("else if test failed");
} else if (a == 2) {
	alert("if else test failed");
} else {
	a = 4;
}

if ( a == 4) {
	alert("else if test passed");
} else if (a == 1) {
	alert("else if test failed");
} else {
	alert("else if test failed");
}


while (a < 6) {
	a = a + 1;
}

if (a == 6) {
	alert("while test passed");
} else {
	alert("while test failed");
}

var b = 0;
for (a = 5; a > 0; a--) {
	b++;
}

if (a == 0) {
	alert("for test passed");
} else {
	alert("for test failed");
}

do {
	a++;
} while (a != 2);


b = 3;

do {
	b++;
} while (b < 2);

if (a == 2 && b ==4 ) {
	alert("do while test passed");
} else {
	alert("do while test failed");
}


alert("done");

return 0;
