# BLE class

class BLE
  def send_temp_humi(temp, humi)
    notify "TH,#{temp},#{humi}"
  end

  def send_temperature(temp)
    notify "TE,#{temp}"
  end

  def send_humidity(humi)
    notify "HU,#{humi}"
  end

  def send_color(r, g, b)
    notify "CO,#{r},#{g},#{b}"
  end

  def send_water_level(wl)
    notify "WL,#{wl}"
  end

  def send(val)
    val = val.to_s unless val.class == String
    notify val
  end
end
