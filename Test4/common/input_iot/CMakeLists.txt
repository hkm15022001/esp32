set(app_src input_iot.c)
set(pri_req)

					
idf_component_register(SRC_DIRS "."
    REQUIRES driver
    INCLUDE_DIRS ".")