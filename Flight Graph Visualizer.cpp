// Flight Graph Visualizer
// Copyright(C) 2024 Nicholas Crepeau
// https://github.com/nickcrepeau/Self-Service-Point-of-Sale

/* This program is free software : you can redistribute it and /or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see < https://www.gnu.org/licenses/>. */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <cstring>
#include "plplot\plstream.h"
using namespace std;

double findDDX(double tCalc, double vxCalc, double mass, double lift, double gamma)
{
	return (1 / mass) * (-lift * sin(gamma));
}

double findDDZ(double tCalc, double vzCalc, double mass, double lift, double gamma, double gravity)
{
	return (1 / mass) * (lift * cos(gamma)) - gravity;
}

int main()
{
	double area = 40, slope = 5, aspectRatio = 6, pi = 3.14159265358979323846, gravity = 9.81, mass = 15000, cdStart = 0.06, efficiency = 0.8, angle = 0.0495, rho, gamma, vInf, lift, xCalc = 0, vzCalc = 0, zCalc = 4800, vxCalc = 210, tCalc = 0, kx = 0, kx1 = 0, kx2 = 0, kx3 = 0, kx4 = 0, kz = 0, kz1 = 0, kz2 = 0, kz3 = 0, kz4 = 0;
	double xArray[401], zArray[401], vxArray[401], vzArray[401], vInfArray[401];
	int dTime = 1, time = 400;

	xArray[0] = 0;
	zArray[0] = 0;

    double cl = slope * angle;
    double cd = cdStart + (pow(cl, 2) / (pi * aspectRatio * efficiency));
	//double drag = 0.5 * rho * pow(vInf, 2) * area * cd;  not used

	for (int i = 0; i <= time; i += dTime)
	{
		rho = pow(1.225, -0.0001 * vzCalc);
		gamma = atan(vzCalc / vxCalc);
		vInf = sqrt(pow(vxCalc, 2) + pow(vzCalc, 2));
		lift = 0.5 * rho * pow(vInf, 2) * area * cl;
		kx1 = findDDX(tCalc, vxCalc, mass, lift, gamma);
		kz1 = findDDZ(tCalc, vzCalc, mass, lift, gamma, gravity);
		kx2 = findDDX(tCalc + dTime / 2, vxCalc + kx1 / 2, mass, lift, gamma);
		kz2 = findDDZ(tCalc + dTime / 2, vzCalc + kz1 / 2, mass, lift, gamma, gravity);
		kx3 = findDDX(tCalc + dTime / 2, vxCalc + kx2 / 2, mass, lift, gamma);
		kz3 = findDDZ(tCalc + dTime / 2, vzCalc + kz2 / 2, mass, lift, gamma, gravity);
		kx4 = findDDX(tCalc, vxCalc + kx3, mass, lift, gamma);
		kz4 = findDDZ(tCalc, vzCalc + kz3, mass, lift, gamma, gravity);
		kx = (kx1 + 2 * kx2 + 2 * kx3 + kx4) / 6;
		kz = (kz1 + 2 * kz2 + 2 * kz3 + kz4) / 6;
		vxCalc += kx;
		vzCalc += kz;
		tCalc = i;
		vxArray[i] = vxCalc;
		vzArray[i] = vzCalc;
		vInfArray[i] = vInf;
		if (i != 0)
		{
			xArray[i] = xArray[i - 1] + vxCalc;
			zArray[i] = zArray[i - 1] + vzCalc;
		}
	}

	int selection = 0;

	cout << "===  Choose a plot to display  === " << endl << "(1) Altitude vs Horizontal Distance" << endl << "(2) Altitude vs Time" << endl << "(3) Flight Speed vs Time" << endl << endl << "Please enter a 1, 2, or 3: ";
	cin >> selection;

	while (selection != 1 && selection != 2 && selection != 3)
	{
		cout << "Error, please enter a 1, 2, or 3: ";
		cin >> selection;
	}

	ofstream outputFile;
	outputFile.open("output.txt");

	outputFile << "time (s)        x (m)       z (m)   Vx (m/s)    Vz (m/s)" << endl;

	for (int i = 0; i <= time; i += 5)
	{
		outputFile << fixed;
		outputFile << setprecision(2);
		outputFile << right;
		outputFile << setw(8);
		outputFile << i;
		outputFile << setw(13);
		outputFile << xArray[i];
		outputFile << setw(12);
		outputFile << zArray[i];
		outputFile << setw(11);
		outputFile << vxArray[i];
		outputFile << setw(12);
		outputFile << vzArray[i] << endl;
	}

	outputFile.close();

	cout << endl << "NOTE: x, z, Vx, and Vz have been written to output.txt" << endl;
	cout << endl << "Plotting..." << endl;

	PLFLT x[401], y[401];
	PLFLT xmin = 0.0, xmax = 400.0, ymin = 0.0, ymax = 350.0;
	auto pls = new plstream();
	plsdev("wingcc");
	pls->init();
	if (selection == 1)
	{
		for (int i = 0; i <= time; i += dTime)
		{
			x[i] = xArray[i];
			y[i] = zArray[i];
		}
		xmin = 0.0;
		xmax = 62000.0;
		ymin = -1200.0;
		ymax = 3600.0;
	}
	if (selection == 2)
	{
		for (int i = 0; i <= time; i += dTime)
		{
			x[i] = i;
			y[i] = zArray[i];
		}
		ymin = -1200.0;
		ymax = 3600.0;
	}
	if (selection == 3)
	{
		for (int i = 0; i <= time; i += dTime)
		{
			x[i] = i;
			y[i] = vInfArray[i];
		}
	}
	pls->env(xmin, xmax, ymin, ymax, 0, 0);
	if (selection == 1)
	{
		pls->col0(3);
		pls->lab("Horizontal Distance (meters)", "Altitude (meters)", "Altitude vs Horizontal Distance");
	}
	if (selection == 2)
	{
		pls->col0(3);
		pls->lab("Time (seconds)", "Altitude (meters)", "Altitude vs Time");
	}
	if (selection == 3)
	{
		pls->col0(3);
		pls->lab("Time (seconds)", "Flight Speed (meters / second)", "Flight Speed vs Time");
	}
	pls->col0(3);
	pls->line(401, x, y);
	delete pls;

    return 0;
}
