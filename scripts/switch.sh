/interface bridge add name=bridge40
/interface bridge add name=bridge41
/interface bridge port remove [find interface=ehter2]
/interface bridge port remove [find interface=ehter3]
/interface bridge port remove [find interface=ehter4]
/interface bridge port remove [find interface=ehter5]
/interface bridge port remove [find interface=ehter6]
/interface bridge port add bridge=bridge40 interface=ehter3
/interface bridge port add bridge=bridge40 interface=ehter4
/interface bridge port add bridge=bridge41 interface=ehter2
/interface bridge port add bridge=bridge41 interface=ehter5
/interface bridge port add bridge=bridge41 interface=ehter6
