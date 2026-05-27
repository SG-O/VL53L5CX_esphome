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
    vl53l_mz_ns,
    VL53LMZ,
    CONF_VL53LMZ_ID,
)

VL53LMZSensor = vl53l_mz_ns.class_(
    "VL53LMZSensor", sensor.Sensor, cg.Component
)

CONF_ZONE_MODE = "zone_mode"
VL53LMZZoneMode = vl53l_mz_ns.enum("VL53LMZZoneMode")
ZONE_MODES = {
    "SINGLE": VL53LMZZoneMode.VL53LMZ_SINGLE,
    "AVERAGE": VL53LMZZoneMode.VL53LMZ_AVERAGE,
    "NEAREST": VL53LMZZoneMode.VL53LMZ_NEAREST,
    "FARTHEST": VL53LMZZoneMode.VL53LMZ_FARTHEST,
    "CENTER": VL53LMZZoneMode.VL53LMZ_CENTER,
}

CONF_ZONE_DATA = "zone_data"
VL53LMZZoneData = vl53l_mz_ns.enum("VL53LMZZoneData")
ZONE_DATA = {
    "DISTANCE": VL53LMZZoneData.VL53LMZ_DISTANCE,
    "SIGMA": VL53LMZZoneData.VL53LMZ_SIGMA,
    "REFLECTANCE": VL53LMZZoneData.VL53LMZ_ZONE_REFLECTANCE,
    "TARGET_COUNT": VL53LMZZoneData.VL53LMZ_ZONE_TARGET_COUNT,
}

CONF_SELECTED_ZONE = "selected_zone"

CONFIG_SCHEMA = cv.All(
    sensor.sensor_schema(
        VL53LMZSensor,
        unit_of_measurement=UNIT_METER,
        icon=ICON_ARROW_EXPAND_VERTICAL,
        accuracy_decimals=3,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.GenerateID(CONF_VL53LMZ_ID): cv.use_id(VL53LMZ),
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
    parent = await cg.get_variable(config[CONF_VL53LMZ_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_zone_mode(config[CONF_ZONE_MODE]))
    cg.add(var.set_zone_data(config[CONF_ZONE_DATA]))
    cg.add(var.set_selected_zone(config[CONF_SELECTED_ZONE]))
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    cg.add(parent.register_sensor(var))
