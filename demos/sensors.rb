# Read data from grove sensors and send data via BLE.

$ble = BLE.new
$ths = GroveTempHumiSensor.new
$cs = GroveColorSensor.new
$wls = GroveWaterLevelSensor.new

LCD.text_size = 2

loop {
  v = $ths.read
  puts "temp:#{v[0]}, humi:#{v[1]}"
  $ble.send_temp_humi(v[0], v[1])
  delay 1000

  v = $cs.read
  puts "red:#{v[0]}, green:#{v[1]}, blue:#{v[2]}"
  $ble.send_color(v[0], v[1], v[2])
  delay 1000

  v = $wls.read
  puts "water level:#{v}"
  $ble.send_water_level(v)
  delay 1000

  LCD.clear
}
