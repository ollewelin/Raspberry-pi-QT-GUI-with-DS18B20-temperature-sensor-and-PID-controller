import tinytuya

#Example to control the Lyfco Heatpump HP050-DC-W to only heater mode  and maybe also HP030-DC-W device
#You need ID, IP adress and LocalKey 
#d = tinytuya.OutletDevice('<ID>xxxxxxxxxxxxxxxxxxxx', '<IP>192.168.x.xxx', '<LocalKey>xxxxxxxxxxxxxxxx')

d = tinytuya.OutletDevice('xxxxxxxxxxxxxxxxxxxx', '192.168.x.xxx', 'xxxxxxxxxxxxxxxx')

d.set_version(3.3)
data = d.status() 
d.set_socketPersistent(True) 
print('set_status() result %r' % data)
d.set_value(4,'hot')

