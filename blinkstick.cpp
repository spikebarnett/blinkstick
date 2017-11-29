#include "blinkstick.hpp"
#include <iomanip>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <png++/png.hpp>
#include <queue>
#include <exception>
#include <sys/stat.h>
#include <libusb-1.0/libusb.h>
#include <iostream>
#include <string>

using namespace std;

bool fileExists(const std::string& file) {
	struct stat buf;
	return (stat(file.c_str(), &buf) == 0);
}

int main(int argc, char* argv[]) {
	blinkstick bs; //bs_rgb rgb;
	
	queue<string> args;
	for(int i=1; i<argc; ++i) args.push(argv[i]);
	
	string png=""; unsigned int delay=10000;
	
	while(!args.empty()) {
		if(args.front()=="--png") {
			args.pop();
			if(!args.empty()) {
				png=args.front();
				args.pop();
			} else {
				cerr << "No argument supplied after `--png`. Aborting\n";
				return 1;
			}
		} else if(args.front()=="--delay") {
			args.pop();
			if(!args.empty()) {
				try {
					delay=stoi(args.front())*1000;
				}
				catch (exception& e) {
					cerr << "Failed to parse delay. Please input an integer value.\n";
					return 0;
				}
				args.pop();
			} else {
				cerr << "No argument supplied after `--delay`. Aborting\n";
				return 1;
			}
			if(delay<10000) {
				delay=10000;
				cerr << "Delay<10ms. Minimum delay is 10ms.\n";
			}
		} else {
			cerr << "Did not understand argument `" << args.front() <<"`. Ignoring it.\n";
			args.pop();
		}
	}

	if(png=="") {
		cerr << "Could not find png file in argument list!\n";
		return 1;
	}
	
	if(!fileExists(png)) {
		cerr << png << ": No such file or directory\n";
		return 0;
	}

	png::image< png::rgb_pixel > image(png);
	if(image.get_width()>=8) {
		for(png::uint_32 y = 0; y < image.get_height(); ++y) {
			unsigned char cmd[26]; cmd[0]=6; cmd[1]=0;
			for (png::uint_32 x = 0; x < 8; ++x) {
				cmd[2+(x*3)] = image[y][x].green;
				cmd[3+(x*3)] = image[y][x].red;
				cmd[4+(x*3)] = image[y][x].blue;
			}
			bs.send_report6(cmd);
		}
	} else {
		cerr << "Image must be at least 8 pixels wide\n";
	}
}