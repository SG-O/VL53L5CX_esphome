import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import (
    CONF_ID,
    ICON_ARROW_EXPAND_VERTICAL,
)
from .. import (
    vl53l5cx_ns,
    VL53L5CX,
    CONF_VL53L5CX_ID,
)

VL53L5CXTextSensor = vl53l5cx_ns.class_(
    "VL53L5CXTextSensor", text_sensor.TextSensor, cg.Component
)

CONF_OUTPUT_FORMATTING = "output_formatting"
VL53L5CXOutputFormatting = vl53l5cx_ns.enum("VL53L5CXOutputFormatting")
OUTPUT_FORMATS = {
    "CSV": VL53L5CXOutputFormatting.VL53L5CX_CSV,
    "TABLE": VL53L5CXOutputFormatting.VL53L5CX_TABLE,
    "BINARY": VL53L5CXOutputFormatting.VL53L5CX_BINARY,
    "PNG": VL53L5CXOutputFormatting.VL53L5CX_PNG,
}

CONF_OUTPUT_DATA = "output_data"
VL53L5CXOutputData = vl53l5cx_ns.enum("VL53L5CXOutputData")
OUTPUT_DATA = {
    "RAW_DISTANCE": VL53L5CXOutputData.VL53L5CX_RAW_DISTANCE,
    "FLOAT_DISTANCE": VL53L5CXOutputData.VL53L5CX_FLOAT_DISTANCE,
    "RAW_SIGMA": VL53L5CXOutputData.VL53L5CX_RAW_SIGMA,
    "FLOAT_SIGMA": VL53L5CXOutputData.VL53L5CX_FLOAT_SIGMA,
    "REFLECTANCE": VL53L5CXOutputData.VL53L5CX_REFLECTANCE,
    "TARGET_COUNT": VL53L5CXOutputData.VL53L5CX_TARGET_COUNT,
    "STATUS": VL53L5CXOutputData.VL53L5CX_STATUS,
}

CONF_FLIP_X = "flip_x"
CONF_FLIP_Y = "flip_y"

CONFIG_SCHEMA = cv.All(
    text_sensor.text_sensor_schema(
        VL53L5CXTextSensor,
        icon=ICON_ARROW_EXPAND_VERTICAL,
    )
    .extend(
        {
            cv.GenerateID(CONF_VL53L5CX_ID): cv.use_id(VL53L5CX),
            cv.Optional(CONF_OUTPUT_FORMATTING, default="CSV"): cv.enum(
                OUTPUT_FORMATS, upper=True, space="_"
            ),
            cv.Optional(CONF_OUTPUT_DATA, default="FLOAT_DISTANCE"): cv.enum(
                OUTPUT_DATA, upper=True, space="_"
            ),
            cv.Optional(CONF_FLIP_X, default=False): cv.boolean,
            cv.Optional(CONF_FLIP_Y, default=False): cv.boolean,
        }
    )
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_VL53L5CX_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_output_formatting(config[CONF_OUTPUT_FORMATTING]))
    cg.add(var.set_output_data(config[CONF_OUTPUT_DATA]))
    cg.add(var.set_flip_x(config[CONF_FLIP_X]))
    cg.add(var.set_flip_y(config[CONF_FLIP_Y]))
    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)
    cg.add(parent.register_sensor(var))
