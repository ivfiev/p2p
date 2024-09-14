import os
import select
import subprocess
import traceback
from datetime import datetime
from random import sample, randint, random
from time import sleep

# for large values - sudo sysctl -w net.netfilter.nf_conntrack_max=1000000
num_nodes = 1000
num_neighbors = 20
num_lines = 5

processes = []
nodes = []


def start_node(name):
    p2p = subprocess.Popen(['../build/p2p', name], stdin=subprocess.PIPE, stdout=subprocess.DEVNULL)
    processes.append(p2p)
    nodes.append(name)


def teardown():
    for p in processes:
        p.kill()
    for p in processes:
        p.wait()


def send(node, text):
    nc = subprocess.Popen(['nc', '-N', 'localhost', node], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    nc.stdin.write(text + "\n")
    nc.stdin.close()
    nc.wait()


def recv(node):
    try:
        nc = subprocess.Popen(['nc', '-N', 'localhost', node], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
        nc.stdin.write("hash,1\n")
        nc.stdin.flush()
        os.set_blocking(nc.stdout.fileno(), False)
        ok, _, _ = select.select([nc.stdout.fileno()], [], [], 0.01)
        if ok:
            response = nc.stdout.read()
            return response
        else:
            print(f'Node {node} failed to respond!')
            return ''
    except Exception as e:
        print(f'Node {node} failed to respond!')
        traceback.print_exc()
        return ''
    finally:
        nc.stdin.close()
        nc.wait()


def get_text():
    with open(__file__, 'r') as f:
        code = f.read()
        lines = code.split('\n')
        for i in range(len(lines)):
            lines[i] = f'{str(i).zfill(3)}\t{lines[i]}'.replace(',', '،').rstrip()
        return lines


def run():
    os.environ['P2P_APP'] = '1'
    os.environ['P2P_LOG_LEVEL'] = '0'
    port = 8080
    for i in range(num_nodes):
        start_node(str(port + i))
    for i in range(len(nodes)):
        ns = sample(nodes, randint(0, num_neighbors))
        if ns:
            peer_msg = f"peers,{nodes[i]},{','.join(ns)}"
            send(nodes[i], peer_msg)
    print('Started nodes...')

    text = get_text()
    for i in range(len(nodes)):
        n = randint(0, num_lines)
        if n:
            lines = sample(text, n)
            msg = f"text,1,{'\n'.join(lines)}"
            node = sample(nodes, 1)[0]
            send(node, msg)
    print('Sent text...')

    while True:
        ratios = []
        for n in nodes:
            ratio = float(recv(n).count('\n')) / float(len(text))
            ratios.append(ratio)
        ratios.sort()

        def get_pc(pc):
            return ratios[int(float(len(ratios)) * float(pc) / 100.0)]

        print(
            f'{datetime.now().strftime('%H:%M:%S')}'
            f' - 25th: {get_pc(25):.2}, 50th: {get_pc(50):.2}, 75th: {get_pc(75):.2}, 95th: {get_pc(95):.2}')
        sleep(0.25)


if __name__ == '__main__':
    try:
        run()
    except Exception as e:
        print(e)
        traceback.print_exc()
    finally:
        teardown()
