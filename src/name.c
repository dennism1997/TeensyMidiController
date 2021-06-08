#include <usb_names.h>

#define MANUFACTURER_NAME {'D', 'M'}
#define MANUFACTURER_NAME_LEN 2
#define PRODUCT_NAME {'T', 'e', 'e', 'n', 's', 'y', 'M', 'i', 'd', 'i'}
#define PRODUCT_NAME_LEN 10
struct usb_string_descriptor_struct usb_string_manufacturer_name = {
        2 + MANUFACTURER_NAME_LEN * 2,
        3,
        MANUFACTURER_NAME};

struct usb_string_descriptor_struct usb_string_product_name = {
        2 + PRODUCT_NAME_LEN * 2,
        3,
        PRODUCT_NAME};
