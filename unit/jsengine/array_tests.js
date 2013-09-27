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

function dec_array_constructor()
 {
	alert("Check: Array constructor ");
	var arr = new Array();
	
	var type = typeof arr;
	if (type != "object") {
		alert(" - FAILED!\n");
		return "ERROR: 'var arr = new Array()'\n\t";
	}

	alert(" - PASS\n");
	return "PASS";
}

function dec_array_string()
{
	alert("Check: String array");
	var arr = ["test", "car"];
	
	var sum = typeof arr[1];
	if (sum != "string") {
		alert(" - FAILED!\n");
		return "ERROR: 'var arr = [\"test\", \"string\"]'\n\t";
	}

	alert(" - PASS\n");
	return "PASS";
}

function dec_array_nums()
{
	alert("Check: Num array");
	var arr = [1, 2, 3, 4, 5, 6, 7, 8, 9];
	
	type = arr[0] + arr[8];
	if (type != 10) {
		alert(" - FAILED!\n");
		return "ERROR: 'var arr = [1, 2, 3, 4, 5, 6, 7, 8, 9]'\n\t";
	}

	alert(" - PASS\n");
	return "PASS";
}

function dec_array_mixed()
{
	alert("Check: String & Num array");
	var arr = ["multi", 2, "type", 3, 6, "array"];
	
	var type1 = typeof arr[0];
	var type2 = typeof arr[3];
	if (type1 != "string" && type2 != "number") {
		alert(" - FAILED!\n");
		return "ERROR: 'var arr = '[\"multi\", 2...'\n\t";
	}

	alert(" - PASS\n");
	return "PASS";
}

function dec_array_cond_constructor()
{
	alert("Check: Condensed constructor");
	var arr = new Array("Saab","Volvo","BMW");
	
	var type = typeof arr[1];
	if (type != "string") {
		alert(" - FAILED!\n");
		return "ERROR: 'var arr = new Array(\"Saab\",...'\n\t";
	}

	alert(" - PASS\n");
	return "PASS";
}

function assign_array_item(a)
{
	alert("Check: Assigning new string value to array item");
	var arr = new Array("Saab", "Volvo", "BMW");
	arr[1] = "String";
	alert("\narr[1] = " + arr[1] + "\n");
	if (arr[1] != a) {
		alert(" - FAILED!\n");
		return "ERROR: 'arr[1] = \"String\"'\n\t";
	}

	alert(" - PASS\n");
	return "PASS";
}

function reverse_array()
{
	alert("Check: Array.reverse()");
	var arr = new Array("Saab", "Volvo", "BMW");
	arr.reverse();
	if (arr[0] != "BMW") {
		alert(" - FAILED!\n");
		return "ERROR: 'arr.reverse()'\n\t";
	}

	alert(" - PASS\n");
	return "PASS";
}

function concat_arrays()
{
	alert("Check: Array.concat(array2)");
	var arr1 = new Array("test", "string", "word");
	var arr2 = new Array("car", "true", "dog", "word", "hi", "goodbye");
	var arrays = arr1.concat(arr2);
	if (arrays[3] != arr2[0]) {
		alert(" - FAILED!\n");
		return "ERROR: 'arrays = arr1.concat(arr2)'\n\t";
	}

	alert(" - PASS\n");
	return "PASS";
}

function join_array()
{
	alert("Check: Array.join(\" and \")");
	var arr = new Array("one", "two", "three");
	var joined = arr.join(" and ");
	if (joined != "one and two and three") {
		alert(" - FAILED!\n");
		return "ERROR: 'joined = arr.join(\" and \")'\n\t";
	}

	alert(" - PASS\n");
	return "PASS";
}

function get_last_index()
{
	alert("Check: Array.lastIndexOf()");
	var arr = new Array("this", "is", "array", "array", "array");
	var last = arr.lastIndexOf("array");
	if (last != 4) {
		alert(" - FAILED!\n");
		return "ERROR: 'var last = arr.lastIndexOf(\"array\")'\n\t";
	}

	alert(" - PASS\n");
	return "PASS";
}

function get_indexof()
{
	alert("Check: Array.indexOf()");
	var arr = new Array("this", "is", "array", "array", "array");
	var index = arr.indexOf("array");
	if (index != 2) {
		alert(" - FAILED!\n");
		return "ERROR: 'var last = arr.lastIndexOf(\"array\")'\n\t";
	}

	alert(" - PASS\n");
	return "PASS";
}

error = "\0";
res = dec_array_constructor();
if (res.contains("ERROR"))
	error = res.concat(error);
res = dec_array_string();
if (res.contains("ERROR"))
	error = res.concat(error);

res = dec_array_nums();
if (res.contains("ERROR"))
	error = res.concat(error);

res = dec_array_mixed();
if (res.contains("ERROR"))
	error = res.concat(error);

res = dec_array_cond_constructor();
if (res.contains("ERROR"))
	error = res.concat(error);

res = assign_array_item("String");
if (res.contains("ERROR"))
	error = res.concat(error);

res = reverse_array();
if (res.contains("ERROR"))
	error = res.concat(error);

res = concat_arrays();
if (res.contains("ERROR"))
	error = res.concat(error);

res = join_array();
if (res.contains("ERROR"))
	error = res.concat(error);

res = get_last_index();
if (res.contains("ERROR"))
	error = res.concat(error);

res = get_indexof();
if (res.contains("ERROR"))
	error = res.concat(error);

return error;
