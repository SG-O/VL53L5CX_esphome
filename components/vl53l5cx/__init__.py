import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome import pins
import base64
from esphome.const import (
    CONF_ADDRESS,
    CONF_ID,
    CONF_INTERRUPT_PIN,
    CONF_RESET_PIN,
    CONF_UPDATE_INTERVAL,
)

CODEOWNERS = ["@SG-O"]

DEPENDENCIES = ["i2c"]

MULTI_CONF = True

vl53l5cx_ns = cg.esphome_ns.namespace("vl53l5cx")
VL53L5CX = vl53l5cx_ns.class_(
    "VL53L5CX", cg.PollingComponent, i2c.I2CDevice
)

CONF_VL53L5CX_ID = "vl53l5cx_id"

CONF_RESOLUTION = "resolution"
VL53L5CXResolution = vl53l5cx_ns.enum("VL53L5CXResolution")
RESOLUTIONS = {
    "4X4": VL53L5CXResolution.VL53L5CX_4X4,
    "8X8": VL53L5CXResolution.VL53L5CX_8X8,
}

CONF_RANGING_MODE = "ranging_mode"
VL53L5CXMode = vl53l5cx_ns.enum("VL53L5CXMode")
MODES = {
    "CONTINUOUS": VL53L5CXMode.VL53L5CX_CONTINUOUS,
    "AUTO": VL53L5CXMode.VL53L5CX_AUTO,
}

CONF_TARGET_ORDER = "target_order"
VL53L5CXTargetOrder = vl53l5cx_ns.enum("VL53L5CXTargetOrder")
TARGET_ORDER = {
    "CLOSEST": VL53L5CXTargetOrder.VL53L5CX_CLOSEST,
    "STRONGEST": VL53L5CXTargetOrder.VL53L5CX_STRONGEST,
}

CONF_RANGING_FREQUENCY = "ranging_frequency"
CONF_INTEGRATION_TIME = "integration_time"
CONF_SHARPENING = "sharpening"
CONF_CONTINUOUS_UPDATE = "continuous_update"
CONF_RUN_XTALK_CALIBRATION = "run_xtalk_calibration"
CONF_XTALK_CALIBRATION_REFLECTANCE = "xtalk_calibration_reflectance"
CONF_XTALK_CALIBRATION_DISTANCE = "xtalk_calibration_distance"
CONF_XTALK_CALIBRATION_DATA = "xtalk_calibration_data"
CONF_LP_PIN = "lp_pin"



def check_keys(obj):
    if obj[CONF_ADDRESS] != 0x29 and CONF_LP_PIN not in obj:
        msg = "Address other then 0x29 requires lp_pin definition."
        raise cv.Invalid(msg)
    if obj[CONF_RESOLUTION] == "8X8" and obj[CONF_RANGING_FREQUENCY] > 15:
        msg = "The ranging frequency must be 15 Hz or less when the resolution is 8X8."
        raise cv.Invalid(msg)
    if obj[CONF_RUN_XTALK_CALIBRATION] and CONF_XTALK_CALIBRATION_DATA in obj:
        msg = "xtalk calibration data must not be defined when running xtalk calibration."
        raise cv.Invalid(msg)
    if obj[CONF_UPDATE_INTERVAL] < cv.TimePeriod(seconds=1):
        msg = "Update interval must not be less than 1s. Use continuous_update: true instead."
        raise cv.Invalid(msg)
    return obj

def check_xtalk_calibration(obj):
    msg = ""
    if obj[CONF_RUN_XTALK_CALIBRATION] and CONF_XTALK_CALIBRATION_DATA in obj:
        msg = "xtalk calibration data must not be defined when running xtalk calibration."
    if CONF_XTALK_CALIBRATION_DATA in obj:
        try:
            decoded = base64.b64decode(obj[CONF_XTALK_CALIBRATION_DATA], validate=True)
            if len(decoded) != 776:
                msg = "invalid xtalk calibration data."
        except Exception:
            msg = "xtalk_calibration_data must be a valid base64 string."
    if msg != "":
        raise cv.Invalid(msg)
    return obj



CONFIG_SCHEMA = cv.All(
    i2c.i2c_device_schema(0x29).extend(
        {
            cv.GenerateID(): cv.declare_id(VL53L5CX),
            cv.Optional(CONF_RESOLUTION, default="8X8"): cv.enum(
                RESOLUTIONS, upper=True, space="_"
            ),
            cv.Optional(CONF_RANGING_FREQUENCY, default="1Hz"): cv.All(
                cv.frequency,
                cv.Range(
                    min=cv.frequency("1Hz"),
                    max=cv.frequency("60Hz"),
                ),
            ),
            cv.Optional(CONF_RANGING_MODE, default="AUTO"): cv.enum(
                MODES, upper=True, space="_"
            ),
            cv.Optional(CONF_INTEGRATION_TIME, default="5ms"): cv.All(
                cv.positive_time_period_microseconds,
                cv.Range(
                    min=cv.TimePeriod(milliseconds=2),
                    max=cv.TimePeriod(milliseconds=249),
                ),
            ),
            cv.Optional(CONF_SHARPENING, default=14): cv.percentage_int,
            cv.Optional(CONF_TARGET_ORDER, default="STRONGEST"): cv.enum(
                TARGET_ORDER, upper=True, space="_"
            ),
            cv.Optional(CONF_CONTINUOUS_UPDATE, default=False): cv.boolean,
            cv.Optional(CONF_RUN_XTALK_CALIBRATION, default=False): cv.boolean,
            cv.Optional(CONF_XTALK_CALIBRATION_REFLECTANCE, default=3): cv.All(
                cv.percentage_int,
                cv.Range(
                    min=cv.percentage_int(1),
                    max=cv.percentage_int(99),
                ),
            ),
            cv.Optional(CONF_XTALK_CALIBRATION_DISTANCE, default="600mm"): cv.All(
                cv.distance,
                cv.Range(
                    min=cv.distance("0.6m"),
                    max=cv.distance("3.0m"),
                ),
            ),
            cv.Optional(CONF_XTALK_CALIBRATION_DATA): cv.string_strict,
            cv.Optional(CONF_RESET_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_INTERRUPT_PIN): pins.internal_gpio_input_pin_schema,
            cv.Optional(CONF_LP_PIN): pins.gpio_output_pin_schema,
        }
    ).extend(cv.polling_component_schema("60s")),
    check_keys,
    check_xtalk_calibration,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    cg.add(var.set_resolution(config[CONF_RESOLUTION]))
    cg.add(var.set_ranging_frequency(config[CONF_RANGING_FREQUENCY]))
    cg.add(var.set_ranging_mode(config[CONF_RANGING_MODE]))
    cg.add(var.set_integration_time(config[CONF_INTEGRATION_TIME]))
    cg.add(var.set_sharpening(config[CONF_SHARPENING]))
    cg.add(var.set_target_order(config[CONF_TARGET_ORDER]))
    cg.add(var.set_continuous_update(config[CONF_CONTINUOUS_UPDATE]))
    if config[CONF_RUN_XTALK_CALIBRATION]:
        cg.add_define("RUN_XTALK_CALIBRATION")
        cg.add(var.set_xtalk_calibration_reflectance(config[CONF_XTALK_CALIBRATION_REFLECTANCE]))
        cg.add(var.set_xtalk_calibration_distance(int(config[CONF_XTALK_CALIBRATION_DISTANCE] * 1000)))
    if CONF_XTALK_CALIBRATION_DATA in config:
        cg.add_define("SET_XTALK_CALIBRATION_DATA")
        cg.add(var.set_xtalk_calibration_data(config[CONF_XTALK_CALIBRATION_DATA]))
    if CONF_RESET_PIN in config:
        reset = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
        cg.add(var.set_reset_pin(reset))
    if CONF_INTERRUPT_PIN in config:
        int_pin = await cg.gpio_pin_expression(config[CONF_INTERRUPT_PIN])
        cg.add(var.set_int_pin(int_pin))
    if CONF_LP_PIN in config:
        lp = await cg.gpio_pin_expression(config[CONF_LP_PIN])
        cg.add(var.set_lp_pin(lp))

    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
