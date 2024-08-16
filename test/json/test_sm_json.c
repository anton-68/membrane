/* SM.EXEC <http://dx.doi.org/10.13140/RG.2.2.12721.39524>
The Membrane (C) project
-------------------------------------------------------------------------------
Copyright 2009-2024 Anton Bondarenko <anton.bondarenko@gmail.com>
-------------------------------------------------------------------------------
SPDX-License-Identifier: LGPL-3.0-only */

#include "../../lib/parson/parson.h"
#include <stdio.h>

int main() {
    char *serialized_string = NULL;
    JSON_Value *root_value0 = json_value_init_object();
    JSON_Object *root_object0 = json_value_get_object(root_value0);
    json_object_set_string(root_object0, "name", "John Smith");
    json_object_set_number(root_object0, "age", 18);
    JSON_Value *root_value1 = json_value_init_object();
    JSON_Object *root_object1 = json_value_get_object(root_value1);
    json_object_set_string(root_object1, "No.", "12345678");
    json_object_set_string(root_object1, "valid till", "2022");
    json_object_set_value(root_object0, "passport", root_value1);
    json_object_dotset_string(root_object0, "address.city", "Cupertino");
    json_object_dotset_value(root_object0, "contact.emails", json_parse_string("[\"email@example.com\",\"email2@example.com\"]"));
    serialized_string = json_serialize_to_string_pretty(root_value0);
    puts(serialized_string);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value0);
    // json_value_free(root_value1); -done with the father's object
    return 0;
}
