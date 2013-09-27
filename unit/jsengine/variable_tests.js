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

function var_dec_num()
{
	alert("Check: Variable declaration - number\n");
	var a = 0;
	if (a != 0)
		return "ERROR: could not initialize 'var a = 0'\n";
	return "PASS\n";
}

function novar_dec_num()
{
	alert("Check: Variable declaration no (var) keyword - number\n");
	b = 3;
	if (b != 3)
		return "ERROR: could not initialize 'b = 3'\n\t";
	return "PASS\n";
}

function var_dec_string()
{
	alert("Check: Variable declaration - string\n");
	var c = "hello";
	if (c == "hedllo")
		return "ERROR: could not initialize 'var c = \"hello\"'\n\t";
	return "PASS\n";
}

function novar_dec_string()
{
	alert("Check: Variable declaration no (var) keyword - string\n");
	d = "world";
	if (d == "worsld")
		return "ERROR: could not initialize 'd = \"world\"'\n\t";
	return "PASS";
}

var error = new String("");
var res = var_dec_num();
if (res.contains("ERROR"))
	error = res;

res = novar_dec_num();
if (res.contains("ERROR"))
	error = res.concat(error);

res = var_dec_string();
if (res.contains("ERROR"))
	error = res.concat(error);

res = novar_dec_string();
if (res.contains("ERROR"))
	error = res.concat(error);

return error;
