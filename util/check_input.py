import schemdraw
from schemdraw import elements as elm
from schemdraw import logic

import matplotlib.pyplot as plt

import networkx as nx
import re

#######################
# Drawing a schematic #
#######################
def cmos_inv_gate(A=0):
    d = schemdraw.Drawing()
    # nmos instance
    nfet = elm.NFet().reverse()
    # pmos instance
    pfet = elm.PFet().reverse()

    # draw from the top
    d.push()
    d += pfet
    d.pop()
    # draw vdd on the top
    vdd_dot = elm.Dot().at(pfet.source)
    d += vdd_dot
    d += elm.oneterm.Vdd().at(pfet.source).label('VDD:1', loc='left')
    d += elm.Label().label('drain').at(pfet.drain).right()
    d += elm.Label().label('source').at(pfet.source).right()
    d += elm.Label().label('gate').at(pfet.gate).left()

    # d.move_from(pfet.drain, 0, -d.unit)
    d.move_from(pfet.drain, 0, 0)
    d.push()
    d += nfet
    d.pop()
    d += elm.Label().label('drain').at(nfet.drain).right()
    d += elm.Label().label('source').at(nfet.source).right()
    d += elm.Label().label('gate').at(nfet.gate).left()
    vss_dot = elm.Dot().at(nfet.source)
    d += vss_dot
    d += elm.oneterm.Vss().at(nfet.source).label('VSS:0', loc='left')

    # draw output
    d.move_from(pfet.drain, 0, 0)
    output_dot = elm.Dot()
    d += output_dot
    d += elm.Line().length(d.unit/2).right().label(f'Y', loc='right')

    # draw input
    d.move_from(nfet.gate, 0, 0)
    d += elm.Line().length(d.unit/4).left()
    d.move_from(pfet.gate, 0, 0)
    d += elm.Line().length(d.unit/4).left()

    # connect the two gates
    d.push()
    d += elm.Line().length(d.unit/2).down()

    # draw the input
    d.pop()
    d.move_from(d.here, 0, -d.unit/4)
    input_dot = elm.Dot()
    d += input_dot
    d += elm.Line().length(d.unit/4).left().label(f'A', loc='left')
    return d

def cmos_and_gate(A=0, B=0):
    d = schemdraw.Drawing()
    
    # nmos instance
    nfet_A = elm.NFet().reverse()
    nfet_B = elm.NFet().reverse()
    # pmos instance
    pfet_A = elm.PFet().reverse()
    pfet_B = elm.PFet().reverse()

    pmos_list = [pfet_A, pfet_B]
    nmos_list = [nfet_A, nfet_B]

    mos_separation = d.unit
    # draw from the top
    for pfet_idx in range(len(pmos_list)):
        d.push()
        d.move_from(d.here, mos_separation*(len(pmos_list)/2 - pfet_idx - 1), 0)
        d += pmos_list[pfet_idx]
        d += elm.Label().label('drain').at(pmos_list[pfet_idx].drain).right()
        d += elm.Label().label('source').at(pmos_list[pfet_idx].source).right()
        d += elm.Label().label('gate').at(pmos_list[pfet_idx].gate).left()
        d += elm.Label().label(f'Inst_{pfet_idx}').at(pmos_list[pfet_idx].center).right()
        d.pop()
    
    # d += elm.Wire('-').at(pmos_list[0].drain).to(pmos_list[1].drain)
    ref_middle_dot = elm.Dot()
    # iterate over the pmos_list and connect all the drains
    for pfet_idx in range(len(pmos_list)-1):
        d.push()
        d += elm.Wire('-').at(pmos_list[pfet_idx].drain).to(pmos_list[pfet_idx+1].drain)
        if pfet_idx == len(pmos_list)//2 - 1:
            print(f"drawing a dot at the middle {pfet_idx}")
            d.move_from(d.here, mos_separation/2, 0)
            d += ref_middle_dot
        d.pop()

    # iterate over the pmos_list and connect all the sources with vdd
    for pfet_idx in range(len(pmos_list) - 1):
        d.push()
        d += elm.Wire('-').at(pmos_list[pfet_idx].source).to(pmos_list[pfet_idx + 1].source).label(f'VDD:{pfet_idx}', loc='left')
        d.pop()

    return d


#######################
#  Reading a netlist  #
#######################
class InstanceInfo:
    def __init__(self, inst_id, inst_type, inst_width):
        self.inst_id = inst_id
        self.inst_type = inst_type
        self.inst_width = inst_width

class PinInfo:
    def __init__(self, pin_id, net_id, inst_id, pin_name, pin_direction, pin_length):
        self.pin_id = pin_id
        self.net_id = net_id
        self.inst_id = inst_id
        self.pin_name = pin_name
        self.pin_direction = pin_direction
        self.pin_length = pin_length

class NetInfo:
    def __init__(self, net_id, net_type, pin_list):
        self.net_id = net_id
        self.net_type = net_type
        self.pin_list = pin_list

def read_netlist(netlist_path):
    '''
    Read the netlist file and return a list of InstanceInfo, PinInfo, NetInfo
    :param netlist_path: path to the netlist file
    :return: instance_info, pin_info, net_info
    '''

    # Read the netlist
    netlist_file = open(netlist_path, 'r')
    netlist = netlist_file.readlines()
    netlist_file.close()

    # set reading mode: InstanceInfo, PinInfo, NetInfo
    mode = 'InstanceInfo'
    instance_info = []
    pin_info = []
    net_info = []

    for line in netlist:
        if line.startswith("i   ==="):
            if line.split()[1] == '===InstanceInfo===':
                mode = 'InstanceInfo'
            elif line.split()[1] == '===PinInfo===':
                mode = 'PinInfo'
            elif line.split()[1] == '===NetInfo===':
                mode = 'NetInfo'
            else:
                print('Error: Invalid mode')
        elif line.startswith("i   InstID"):
            continue
        elif line.startswith("i   PinID"):
            continue
        elif line.startswith("i   NetID"):
            continue
        elif line.startswith("i   "):
            if mode == 'InstanceInfo':
                inst_id = line.split()[1]
                inst_type = line.split()[2]
                inst_width = line.split()[3]
                instance_info.append(InstanceInfo(inst_id, inst_type, inst_width))
            elif mode == 'PinInfo':
                pin_id = line.split()[1]
                net_id = line.split()[2]
                inst_id = line.split()[3]
                pin_name = line.split()[4]
                pin_direction = line.split()[5]
                pin_length = line.split()[6]
                pin_info.append(PinInfo(pin_id, net_id, inst_id, pin_name, pin_direction, pin_length))
            elif mode == 'NetInfo':
                net_id = line.split()[1]
                net_type = line.split()[2]
                pin_list = line.split()[3:]
                net_info.append(NetInfo(net_id, net_type, pin_list))
            else:
                print('Error: Invalid mode')

    return instance_info, pin_info, net_info

def generate_netlist_graph_for_pmos(net_info, instance_info, pin_info):
    '''
    Generate a graph from the netlist for PMOS
    :param net_info: net_info
    :param instance_info: instance_info
    :param pin_info: pin_info
    :return: netlist_graph
    '''
    netlist_graph = nx.MultiGraph()

    # add nodes for VDD and output
    # netlist_graph.add_node('VDD')
    # netlist_graph.add_node('output')

    # get the power pin name
    vdd_pin_id = ''
    vss_pin_id = ''
    input_pin_id = []
    output_pin_id = ''

    pmos_pin_id = []
    for pin in pin_info:
        if pin.pin_name == 'VDD':
            vdd_pin_id = pin.pin_id
        elif pin.pin_name == 'VSS':
            vss_pin_id = pin.pin_id
        elif pin.pin_name in ['I', 'A', 'B', 'A1', 'A2', 'A3', 'A4', 'B1', 'B2', 'B3', 'B4']:
            input_pin_id.append(pin.pin_id)
        elif pin.pin_name in ['O', 'Y', 'Z', 'Q', 'ZN']:
            output_pin_id = pin.pin_id

        # if pin inst_type is PMOS, add the pin_id to pmos_pin_id
        if pin.inst_id in [inst.inst_id for inst in instance_info if inst.inst_type == 'PMOS']:
            pmos_pin_id.append(pin.pin_id)

    print(f"input_pin_names: {input_pin_id}")

    for net in net_info:
        # skip any input nets
        if net.pin_list[0] in input_pin_id:
            continue
        # skip creating nodes for power nets
        elif net.pin_list[0] == vss_pin_id:
            continue
        # add nodes for internal nets
        elif net.pin_list[0] in pmos_pin_id:
            netlist_graph.add_node(net.net_id)
    
    for inst in instance_info:
        # skip non-pmos instances
        if inst.inst_type != 'PMOS':
            continue

        # extract the source and drain pin ids
        source_pin_id = ''
        source_pin_net_id = ''
        drain_pin_id = ''
        drain_pin_net_id = ''
        for pin in pin_info:
            if pin.inst_id == inst.inst_id:
                if pin.pin_name == 'S':
                    source_pin_id = pin.pin_id
                    source_pin_net_id = pin.net_id
                elif pin.pin_name == 'D':
                    drain_pin_id = pin.pin_id
                    drain_pin_net_id = pin.net_id

        print(f"source_pin_id: {source_pin_id}, source_pin_net_id: {source_pin_net_id}")
        print(f"drain_pin_id: {drain_pin_id}, drain_pin_net_id: {drain_pin_net_id}")

        # add edges for the source and drain
        netlist_graph.add_edge(source_pin_net_id, drain_pin_net_id, weight=int(re.findall(r'\d+', inst.inst_width)[0]))
    return netlist_graph

def generate_netlist_graph_for_nmos(net_info, instance_info, pin_info):
    '''
    Generate a graph from the netlist for NMOS
    :param net_info: net_info
    :param instance_info: instance_info
    :param pin_info: pin_info
    :return: netlist_graph
    '''
    netlist_graph = nx.MultiDiGraph()

    # add nodes for VDD and output
    # netlist_graph.add_node('VDD')
    # netlist_graph.add_node('output')

    # get the power pin name
    vdd_pin_id = ''
    vss_pin_id = ''
    input_pin_id = []
    output_pin_id = ''

    nmos_pin_id = []
    for pin in pin_info:
        if pin.pin_name == 'VDD':
            vdd_pin_id = pin.pin_id
        elif pin.pin_name == 'VSS':
            vss_pin_id = pin.pin_id
        elif pin.pin_name in ['I', 'A', 'B', 'A1', 'A2', 'A3', 'A4', 'B1', 'B2', 'B3', 'B4']:
            input_pin_id.append(pin.pin_id)
        elif pin.pin_name in ['O', 'Y', 'Z', 'Q', 'ZN']:
            output_pin_id = pin.pin_id

        # if pin inst_type is PMOS, add the pin_id to pmos_pin_id
        if pin.inst_id in [inst.inst_id for inst in instance_info if inst.inst_type == 'NMOS']:
            nmos_pin_id.append(pin.pin_id)

    print(f"input_pin_names: {input_pin_id}")

    for net in net_info:
        # skip any input nets
        if net.pin_list[0] in input_pin_id:
            continue
        # skip creating nodes for power nets
        elif net.pin_list[0] == vdd_pin_id:
            continue
        # add nodes for internal nets
        elif net.pin_list[0] in nmos_pin_id:
            netlist_graph.add_node(net.net_id)
    
    for inst in instance_info:
        # skip non-pmos instances
        if inst.inst_type != 'NMOS':
            continue

        # extract the source and drain pin ids
        source_pin_id = ''
        source_pin_net_id = ''
        drain_pin_id = ''
        drain_pin_net_id = ''
        for pin in pin_info:
            if pin.inst_id == inst.inst_id:
                if pin.pin_name == 'S':
                    source_pin_id = pin.pin_id
                    source_pin_net_id = pin.net_id
                elif pin.pin_name == 'D':
                    drain_pin_id = pin.pin_id
                    drain_pin_net_id = pin.net_id

        print(f"source_pin_id: {source_pin_id}, source_pin_net_id: {source_pin_net_id}")
        print(f"drain_pin_id: {drain_pin_id}, drain_pin_net_id: {drain_pin_net_id}")

        # add edges for the source and drain
        netlist_graph.add_edge(source_pin_net_id, drain_pin_net_id, label=inst.inst_id)
    
    return netlist_graph

def get_mos_pairs(net_info, instance_info, pin_info):
    '''
    a pair of PMOS and NMOS is a MOS pair if they have the same input
    :param net_info: net_info
    :param instance_info: instance_info
    :param pin_info: pin_info
    '''
    mos_pair_list = {}
    
    # traverse through the net_info and find the input nets
    input_pin_id = []
    input_net_list = []

    for pin in pin_info:
        if pin.pin_name in ['I', 'A', 'B', 'A1', 'A2', 'A3', 'A4', 'B1', 'B2', 'B3', 'B4']:
            input_pin_id.append(pin.pin_id)
            input_net_list.append(pin.net_id)

    # print(f"input_pin_names: {input_pin_id}")
    # print(f"input_net_list: {input_net_list}")

    # traverse again and find the PMOS and NMOS pairs
    # if the two pins are in the same net, they are a pair
    for pin in pin_info:
        for input_net in input_net_list:
            # if pin net_id is the same as input_net
            if pin.net_id == input_net:
                # if pin inst_id matches insMM#
                if re.match(r'insMM\d+', pin.inst_id):
                    # add the pin_id to the mos_pair_list
                    if input_net in mos_pair_list:
                        mos_pair_list[input_net].append(pin.inst_id)
                    else:
                        mos_pair_list[input_net] = [pin.inst_id]

    print(mos_pair_list)
    

def main():
    path = '/home/dinple/SMTCell/SMTCellUCSD_Ver1.2_wip/inputPinLayouts/pinLayouts_2F_4T/NAND2_X1.pinLayout'
    instance_info, pin_info, net_info = read_netlist(path)

    for inst in instance_info:
        print(inst.inst_id, inst.inst_type, inst.inst_width)

    for pin in pin_info:
        print(pin.pin_id, pin.net_id, pin.inst_id, pin.pin_name, pin.pin_direction, pin.pin_length)

    for net in net_info:
        print(net.net_id, net.net_type, net.pin_list)

    # cmos_inv_gate().draw(showframe=False)
    # cmos_and_gate().draw(showframe=False)

    # # netlist_graph = generate_netlist_graph_for_pmos(net_info, instance_info, pin_info)
    # netlist_graph = generate_netlist_graph_for_pmos(net_info, instance_info, pin_info)

    # # extract node labels
    # node_labels = {}
    # for node in netlist_graph.nodes:
    #     node_labels[node] = node
        
    # print(node_labels)
        
    # # extract edge labels
    # edge_labels = {}
    # for edge in netlist_graph.edges:
    #     edge_labels[edge] = nx.get_edge_attributes(netlist_graph, 'weight')
        
    # print(edge_labels)

    # pos = nx.spring_layout(netlist_graph)
    # nx.draw_networkx_nodes(netlist_graph, pos=pos)
    # nx.draw_networkx_labels(netlist_graph, pos=pos, labels=node_labels)
    # rad = 0.1
    # for e in netlist_graph.edges:
    #     print(e)
    #     nx.draw_networkx_edges(netlist_graph, pos=pos, connectionstyle=f'arc3, rad = {rad * e[2]}', edgelist=[e], arrows=True)
    #     rad *= -1
    # plt.show()

    get_mos_pairs(net_info, instance_info, pin_info)
    
if __name__ == '__main__':
    main()

