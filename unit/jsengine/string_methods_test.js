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

var a = "cool";
var b = "beans";
alert("assigned a and b");
var c = a.concat(b);
alert(a);
alert(b);
alert(c);
c = a.charAt(0);
alert(c);
c = b.charCodeAt(0);
alert(c);
c = b.contains("eans");
alert("should be 1\n");
alert(c+"\n");
c = b.contains("cool");
alert("should be 0\n");
alert(c+"\n");

c = b.contains("ans", 1);
alert("should be 1\n");
alert(c+"\n");
c = b.contains("bean", 1);
alert("should be 0\n");
alert(c+"\n");
alert(b+"\n");
c = b.endsWith("ans");
alert("should be 1\n");
alert(c+"\n");
c = b.endsWith("ans", 3);
alert("should be 0\n");
alert(c+"\n");

c = b.endsWith("ean", 3);
alert("should be 1\n");
alert(c+"\n");
c = b.endsWith("dup");
alert("should be 0\n");
alert(c+"\n");
b = "super cool beans";
c = b.indexOf("cool");
alert("this should be 6\n");
alert(c+"\n");
c = b.indexOf("loop");
alert("this should be -1\n");
alert(c+"\n");
c = b.indexOf("cool", 6);
alert("this should be 0\n");
alert(c+"\n");
c = b.indexOf("super", 6);
alert("this should be -1\n");
alert(c);
b = "pleasework";
c = b.lastIndexOf("e");
alert("this should be 5:");
alert(c);

c = b.lastIndexOf("yup");
alert("this should be -1");
alert(c);
c = b.lastIndexOf("e", 5);
alert("this should be 2:");
alert(c);
c = b.lastIndexOf("k", 2);
alert("this should be -1");
alert(c);
c = b.localCompare("a");
alert("this should not be 0");
alert(c);
b = "a";
c = b.localCompare("a");
alert("this should be 0");
alert(c);
c = b.printHello();
alert(c);
b = "apples and oranges and some more oranges";
c = b.replace("oranges", "bananas");
alert(b);
alert(c);
b = "party tonight!";
c = b.splice(6);
alert("should be tonight!");
alert(c);
c = b.splice(-1);
alert("should be !");
alert(c);
c = b.splice(3, 4);
alert("should be ty");
alert(c);
c = b.splice(0, -1);
alert("should be party tonight");
alert(c);
b = "april may june ";

var c = ["april", "may", "june"];
alert(c);
c = b.split(" ");
alert("split it");
alert(c);
var c = b.substr(6);
alert("should be may june");
alert(c);
c = b.substr(6, 3);
alert("should be may");
alert(c);
c = b.substr(-4, 4);
alert("should be june");
alert(c);
c = b.substring(6);
alert("should be may june");
alert(c);
c = b.substring(6, 9);
alert("should be may");
alert(c);
c = b.substring(5, 0);
alert("should be april");
alert(c);
c = b.substring(-5);
alert("should be april may june");
alert(c);
return 0;
