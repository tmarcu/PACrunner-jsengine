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

alert("*Check: +");
if ((2 + 2) != 4) {
	alert("    Failed!");
}

alert("*Check: -");
if ((2 - 2) != 0) {
	alert("    Failed!");
}

alert("*Check: *");
if ((2 * 2) != 4) {
	alert("    Failed!");
}

alert("*Check: /");
if ((2 / 2) != 1) {
	alert("    Failed!");
}

alert("*Check: ^");
if ((1 ^ 2) != 3) {
	alert("    Failed!");
}

alert("*Check: %");
if ((8%2) != 0) {
	alert("    Failed!");
}

alert("*Check: &");
if ((10 & 2) != 2) {
	alert("    Failed!");
}

alert("*Check: |");
if ((8 | 2) != 10) {
	alert("    Failed!");
}

alert("*Check: >>");
if ((32 >> 2) != 8) {
	alert("    Failed!");
}

alert("*Check: <<");
if ((32 << 1) != 64) {
	alert("    Failed!");
}

alert("*Check: >>");
if ((32 >> 2) != 8) {
	alert("    Failed!");
}

return 0;
