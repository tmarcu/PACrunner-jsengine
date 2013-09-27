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

var res = new String("\0");
var error = new String("\0");
function test_obj_exec ()
{
	var reg_obj = new RegExp("d(b+)(d)", "ig");
	var reg_obj_exec = reg_obj.exec("cdbBdbsbz");
	if (reg_obj_exec[0] != "dbBd")
		return "ERROR: RegExp.exec";

	if (reg_obj_exec[1] != "bB")
		return "ERROR: RegExp.exec";

	if (reg_obj_exec[2] != "d")
		return "ERROR: RegExp.exec";

	if (reg_obj.lastIndex != 5)
		return "ERROR: RegExp.exec lastIndex";

	return "PASSED: OBJ RegExp.exec";
}

function successive_matches()
{
	var reg_obj = new RegExp("ab*", "g");
	var str = "abbcdefabh";
	var result;
	var last_arr = [3, 9];
	var result_arr = ["abb", "ab"];
	var i = 0;
	result = reg_obj.exec(str);

	while (result != null) {
		if (result[0] != result_arr[i]) {
			return "ERROR: successive matches\n";
		}

		if (reg_obj.lastIndex != last_arr[i])
			return "ERROR: successive matches lastIndex\n";

		i++;
		result = reg_obj.exec(str);
	}

	if (reg_obj.lastIndex != 0)
		return "ERROR: successive matches\n";

	return "PASSED: successive matches\n";
}

function check_test()
{
	var reg_obj = new RegExp("\w+\d*", "g");
	if (reg_obj.test("cool541")) {
		return "PASSED: RegExp.test\n";
	} else {
		return "ERROR: RegExp.test\n";
	}
}

function var_reg()
{
	var regex = /(\w+)\s(\w+)/;
	if (!regex.test("two words")) {
		return "ERROR: var regexp test\n";
	}
	var words = ["two", "words"];
	var result = regex.exec("two words");
	var i = 0;

	for (i = 0; i < 2; i++) {
		if (words[i] != result[i + 1])
			return "ERROR: var regexp exec\n";
	}
	return "PASSED: var RegExp\n";
}

function dec_reg()
{
	var result;
	result = /\w+ \w+/g.test("two words");
	if (!result)
		return "ERROR: implicit RegExp test\n";

	result = /\w+ \w+/g.exec("two words");
	if (result[0] != "two words")
		return "ERROR: implicit RegExp exec\n";

	return "PASSED: implicit RegExp\n";
}

function reg_str_match()
{
	var str = "For more information, see Chapter 3.4.5.1";
	var re = /(chapter \d+(\.\d)*)/i;
	var found = str.match(re);
	var should = ["Chapter 3.4.5.1", "Chapter 3.4.5.1", ".1"];
	var i = 0;

	while (i < 3) {
		if (found[i] != should[i])
			return "ERROR: string.match\n";
		i++;
	}

	var str = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	var regexp = /[A-E]/gi;
	var matches_array = str.match(regexp);
	var test_match = ["A", "B", "C", "D", "E", "a", "b", "c", "d", "e"];
	for (i = 0; i < 10; i++) {
		if (test_match[i] != matches_array[i])
			return "ERROR: string.match\n";
	}

	return "PASSED: string.match\n";
}

function reg_str_search()
{
		var str = "the are all words so search this string";
		var reg = /words/g;
		if (str.search(reg))
			return "PASSED: string.search\n";
		return "ERROR: string.search\n";
}

function reg_str_replace()
{
	var str = "'Twas the night before Xmas...";
	var test_str = "'Twas the night before Christmas...";
	var new_str = str.replace(/xmas/i, "Christmas");
	if (new_str != test_str)
		return "ERROR: string.replace\n";

	return "PASSED: string.replace\n";
}


res = test_obj_exec();
error = res;
error = error.concat("\n");
res = successive_matches();
error = error.concat(res);
error = error.concat(check_test());
error = error.concat(var_reg());
error = error.concat(dec_reg());
error = error.concat(reg_str_match());
error = error.concat(reg_str_search());
error = error.concat(reg_str_replace());
alert(error);
return 0;
