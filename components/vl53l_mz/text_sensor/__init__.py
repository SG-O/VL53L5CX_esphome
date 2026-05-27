import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import (
    CONF_ID,
    ICON_ARROW_EXPAND_VERTICAL,
)
from .. import (
    vl53l_mz_ns,
    VL53LMZ,
    CONF_VL53LMZ_ID,
)

VL53LMZTextSensor = vl53l_mz_ns.class_(
    "VL53LMZTextSensor", text_sensor.TextSensor, cg.Component
)

CONF_OUTPUT_FORMATTING = "output_formatting"
VL53LMZOutputFormatting = vl53l_mz_ns.enum("VL53LMZOutputFormatting")
OUTPUT_FORMATS = {
    "CSV": VL53LMZOutputFormatting.VL53LMZ_CSV,
    "TABLE": VL53LMZOutputFormatting.VL53LMZ_TABLE,
    "BINARY": VL53LMZOutputFormatting.VL53LMZ_BINARY,
    "PNG": VL53LMZOutputFormatting.VL53LMZ_PNG,
}

CONF_OUTPUT_DATA = "output_data"
VL53LMZOutputData = vl53l_mz_ns.enum("VL53LMZOutputData")
OUTPUT_DATA = {
    "RAW_DISTANCE": VL53LMZOutputData.VL53LMZ_RAW_DISTANCE,
    "FLOAT_DISTANCE": VL53LMZOutputData.VL53LMZ_FLOAT_DISTANCE,
    "RAW_SIGMA": VL53LMZOutputData.VL53LMZ_RAW_SIGMA,
    "FLOAT_SIGMA": VL53LMZOutputData.VL53LMZ_FLOAT_SIGMA,
    "REFLECTANCE": VL53LMZOutputData.VL53LMZ_REFLECTANCE,
    "TARGET_COUNT": VL53LMZOutputData.VL53LMZ_TARGET_COUNT,
    "STATUS": VL53LMZOutputData.VL53LMZ_STATUS,
}

CONF_FLIP_X = "flip_x"
CONF_FLIP_Y = "flip_y"

CONFIG_SCHEMA = cv.All(
    text_sensor.text_sensor_schema(
        VL53LMZTextSensor,
        icon=ICON_ARROW_EXPAND_VERTICAL,
    )
    .extend(
        {
            cv.GenerateID(CONF_VL53LMZ_ID): cv.use_id(VL53LMZ),
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
    parent = await cg.get_variable(config[CONF_VL53LMZ_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_output_formatting(config[CONF_OUTPUT_FORMATTING]))
    cg.add(var.set_output_data(config[CONF_OUTPUT_DATA]))
    cg.add(var.set_flip_x(config[CONF_FLIP_X]))
    cg.add(var.set_flip_y(config[CONF_FLIP_Y]))
    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)
    cg.add(parent.register_sensor(var))
