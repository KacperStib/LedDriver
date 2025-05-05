set(srcs "main.c")

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "."
                    REQUIRES bh1750 i2c sht40 ina219 ledcDriver pir
                    )
