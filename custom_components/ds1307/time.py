from esphome.components import i2c, time as time_
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID

DEPENDENCIES = ['i2c']

ds1307_ns = cg.esphome_ns.namespace('ds1307')
DS1307Component = ds1307_ns.class_('DS1307Component', time_.RealTimeClock, i2c.I2CDevice)

CONFIG_SCHEMA = time_.TIME_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(DS1307Component),
}).extend(cv.COMPONENT_SCHEMA).extend(i2c.i2c_device_schema(0x68))

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield time_.register_time(var, config)
    yield i2c.register_i2c_device(var, config)
