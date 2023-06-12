def Ccode_input_spike():
    with open('input_spike.txt', 'r') as input_file, open('Ccode_output.txt', 'w') as output_file:
        addr_count = 0
        for line in input_file:
            line = line.rstrip('\n')  # 去除行末的换行符

            # INPUT SPIKES
            line = "  input_spike = 0x" + line + ";"
            addr = "  input_spike_addr = " + str(addr_count) + ";"
            addr_count = addr_count + 1
            write = "  tinyODIN_spike_core_write(&tinyODIN, input_spike_addr, input_spike);"

            output_file.write(line+"\n")  
            output_file.write(addr+"\n")  
            output_file.write(write+"\n") 
            output_file.write("\n") 
    
def Ccode_synapse_l1():
    with open('model_l1.txt', 'r') as input_file, open('Ccode_output.txt', 'w') as output_file:
        addr_count = 0
        begin = "synapse_l1 = {"
        weight = ""
        for line in input_file:
            line = line.rstrip('\n')  # 去除行末的换行符
            # Weight
            weight = weight + "0x" + line + ", "
        write = begin + weight  + "};"

        output_file.write(write+"\n")     

if __name__ == "__main__": 
    Ccode_synapse_l1()


