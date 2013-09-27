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

var a = 3;
var foo = [4, 7, 8];
function dostuff(t) {
	var a = 8;
	alert("should be: 8\n");
	alert(a+"\n");
	var foo = ["cool"];
	alert("should be: cool\n");
	alert(foo[0]+"\n");
	return 7;
}


var b = dostuff(a);
alert("should be: 3");
alert(a);
alert("should be: 4");
b = foo[0];
alert(b);
alert("done with test\n");
return 0;
