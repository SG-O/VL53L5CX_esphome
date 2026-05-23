import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    ICON_ARROW_EXPAND_VERTICAL,
    STATE_CLASS_MEASUREMENT,
    UNIT_METER,
    UNIT_PERCENT,
    UNIT_EMPTY,
    CONF_UNIT_OF_MEASUREMENT,
    CONF_ACCURACY_DECIMALS
)
from .. import (
    vl53l5cx_ns,
    VL53L5CX,
    CONF_VL53L5CX_ID,
)

VL53L5CXSensor = vl53l5cx_ns.class_(
    "VL53L5CXSensor", sensor.Sensor, cg.Component
)

CONF_ZONE_MODE = "zone_mode"
VL53L5CXZoneMode = vl53l5cx_ns.enum("VL53L5CXZoneMode")
ZONE_MODES = {
    "SINGLE": VL53L5CXZoneMode.VL53L5CX_SINGLE,
    "AVERAGE": VL53L5CXZoneMode.VL53L5CX_AVERAGE,
    "NEAREST": VL53L5CXZoneMode.VL53L5CX_NEAREST,
    "FARTHEST": VL53L5CXZoneMode.VL53L5CX_FARTHEST,
    "CENTER": VL53L5CXZoneMode.VL53L5CX_CENTER,
}

CONF_ZONE_DATA = "zone_data"
VL53L5CXZoneData = vl53l5cx_ns.enum("VL53L5CXZoneData")
ZONE_DATA = {
    "DISTANCE": VL53L5CXZoneData.VL53L5CX_DISTANCE,
    "SIGMA": VL53L5CXZoneData.VL53L5CX_SIGMA,
    "REFLECTANCE": VL53L5CXZoneData.VL53L5CX_ZONE_REFLECTANCE,
    "TARGET_COUNT": VL53L5CXZoneData.VL53L5CX_ZONE_TARGET_COUNT,
}

CONF_SELECTED_ZONE = "selected_zone"

CONFIG_SCHEMA = cv.All(
    sensor.sensor_schema(
        VL53L5CXSensor,
        unit_of_measurement=UNIT_METER,
        icon=ICON_ARROW_EXPAND_VERTICAL,
        accuracy_decimals=3,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.GenerateID(CONF_VL53L5CX_ID): cv.use_id(VL53L5CX),
            cv.Optional(CONF_ZONE_MODE, default="SINGLE"): cv.enum(
                ZONE_MODES, upper=True, space="_"
            ),
            cv.Optional(CONF_ZONE_DATA, default="DISTANCE"): cv.enum(
                ZONE_DATA, upper=True, space="_"
            ),
            cv.Optional(CONF_SELECTED_ZONE, default=0): cv.int_range(min=0, max=63),
        }
    )
)


async def to_code(config):
    if config.get(CONF_ZONE_DATA) == "REFLECTANCE":
        config[CONF_UNIT_OF_MEASUREMENT] = UNIT_PERCENT
        config[CONF_ACCURACY_DECIMALS] = 0
    if config.get(CONF_ZONE_DATA) == "TARGET_COUNT":
        config[CONF_UNIT_OF_MEASUREMENT] = UNIT_EMPTY
        config[CONF_ACCURACY_DECIMALS] = 0
    parent = await cg.get_variable(config[CONF_VL53L5CX_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_zone_mode(config[CONF_ZONE_MODE]))
    cg.add(var.set_zone_data(config[CONF_ZONE_DATA]))
    cg.add(var.set_selected_zone(config[CONF_SELECTED_ZONE]))
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    cg.add(parent.register_sensor(var))
