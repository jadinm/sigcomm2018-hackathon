"""This file contains a simple topology for the Hackathon"""

from sr6mininet.cli import SR6CLI
from sr6mininet.sr6net import SR6Net
from ipmininet.iptopo import IPTopo
from sr6mininet.sr6router import SRv6Encap, SRv6Route


class HackathonSRNet(IPTopo):
    def build(self, *args, **kwargs):
        """
                                 +----+
                            +----+ R1 +-----+
                            |    +----+     |
                            |               |
             +--------+  +--+-+          +--+-+    +----+  +--------+
             | client +--+ R2 +----------+ R3 +----+ R4 +--+ server |
             +--------+  +----+          +----+    +----+  +--------+
        """
        r1 = self.addRouter('r1')
        r2 = self.addRouter('r2')
        r3 = self.addRouter('r3')
        r4 = self.addRouter('r4')
        self.addLink(r1, r2)
        self.addLink(r1, r3)
        self.addLink(r3, r2)
        self.addLink(r3, r4)

        client = self.addHost("client")
        server = self.addHost("server")
        self.addLink(client, r2)
        self.addLink(server, r4)

        super(HackathonSRNet, self).build(*args, **kwargs)


net = SR6Net(topo=HackathonSRNet(), static_routing=True)
try:
    net.start()
    SR6CLI(net)
finally:
    net.stop()

