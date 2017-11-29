#include <libusb-1.0/libusb.h>
#include <iostream>
#include <string>

bool DEBUG = true;
const int BLINKSTICK_VEND_ID = 0x20A0;
const int BLINKSTICK_PROD_ID = 0x41E5;

//const int BLINKSTICK_VEND_ID = 8352;  //"0X20A0";
//const int BLINKSTICK_PROD_ID = 16869; //"0X41E5";
const int COLOR_PACKET_SIZE = 26;

void debug(std::string msg) {
	if(DEBUG==true) std::cout << msg;
}

int is_blinkstick(libusb_device *device) {
  struct libusb_device_descriptor desc;
  int result = libusb_get_device_descriptor(device, &desc);

  if (result >= 0) {
    if ((desc.idVendor == BLINKSTICK_VEND_ID) && (desc.idProduct == BLINKSTICK_PROD_ID)) {
      debug("Found the blinkstick\n");
      return true;
    }
  }
  return false;
}

libusb_device_handle* claim_device(libusb_device* device) {
  if (device == NULL) {
    debug("No device to claim\n");
    return NULL;
  }

  libusb_device_handle *dev_handle = NULL;
  int open_device_result;

  open_device_result = libusb_open(device, &dev_handle);

  if (open_device_result >= 0) {
    libusb_claim_interface(dev_handle, 0);
    libusb_ref_device(device);
  } else {
    debug("Failed at claiming device\n");
  }

  return dev_handle;
}

// Only finds first BlinkStick currently.
class blinkstick {
	private:
  libusb_device_handle* handle;
  libusb_context* usb_context;
  unsigned char data[26];
  
  public:
  blinkstick() {
		usb_context = NULL;
		libusb_device **devices;
		ssize_t device_count;
		debug("Initializing USB context\n");
		libusb_init(&usb_context);
		if (DEBUG) {
			libusb_set_debug(usb_context, 3);
		}
		device_count = libusb_get_device_list(usb_context, &devices);
		debug("Found "+std::to_string(device_count)+" usb devices\n");
		libusb_device *blinkstick_dev = NULL;
		
		for(int i = 0; i < device_count; i++) {
			libusb_device *device = devices[i];

			struct libusb_device_descriptor desc;
			int result = libusb_get_device_descriptor(device, &desc);

			if (result >= 0) {
				if ((desc.idVendor == BLINKSTICK_VEND_ID) && (desc.idProduct == BLINKSTICK_PROD_ID)) {
					debug("Found the blinkstick\n");
					blinkstick_dev = device; break;
				}
			}
		}
		handle = claim_device(blinkstick_dev);
		libusb_free_device_list(devices, 1);
		data[0] = 6; data[1] = 0;
		for(unsigned int i = 2; i < COLOR_PACKET_SIZE; ++i) data[i] = 0;
	}
	
	~blinkstick() {
		debug("Releasing the blinkstick\n");
		libusb_close(handle);
		libusb_exit(usb_context);
	}
	
	void set_color(char I, char R, char G, char B) {
		data[2+(I*3)]=G;
		data[3+(I*3)]=R;
		data[4+(I*3)]=B;
	}
	
	void send_report5(unsigned char* DATA) {
		libusb_control_transfer(handle, 0x20, 0x9, 0x0005, 0x0000, DATA,  6, 10);
	}
	
	void send_report6(unsigned char* DATA) {
		libusb_control_transfer(handle, 0x20, 0x9, 0x0006, 0x0000, DATA, 26, 10);
	}
	
	void update(void) {
		libusb_control_transfer(handle, 0x20, 0x9, 0x0006, 0x0000, data, COLOR_PACKET_SIZE, 10);
	}
};