"""Climate entitás az Innova/Ariston B32 fancoil Modbus-regisztereihez.

A Modbus-kommunikációt a YAML interval-lambdája végzi; ez a komponens csak
a regiszterértékekből climate-állapotot számol, és a climate-parancsokat
regiszter-írásokká alakítja (pop_write).
"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate

from . import fancoil_climate_ns

FancoilClimate = fancoil_climate_ns.class_(
    "FancoilClimate", climate.Climate, cg.Component
)

CONFIG_SCHEMA = climate.climate_schema(FancoilClimate).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = await climate.new_climate(config)
    await cg.register_component(var, config)
