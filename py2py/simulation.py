import os
import subprocess
import traceback
from random import sample, randint
from time import sleep

num_nodes = 100

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
    nc = subprocess.Popen(['nc', '-N', 'localhost', node], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    nc.stdin.write("hash,1\n")
    nc.stdin.flush()
    os.set_blocking(nc.stdout.fileno(), False)
    sleep(0.02)
    response = nc.stdout.read()
    nc.stdin.close()
    nc.wait()
    return response


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
        ns = sample(nodes, randint(0, 10))
        if len(ns) > 0:
            peer_msg = f"peers,{nodes[i]},{','.join(ns)}"
            send(nodes[i], peer_msg)
    text = get_text()
    for i in range(len(nodes)):
        n = randint(1, 10)
        lines = sample(text, n)
        msg = f"text,1,{'\n'.join(lines)}"
        node = sample(nodes, 1)[0]
        send(node, msg)
    while True:
        sum = 0.0
        for n in nodes:
            ratio = float(recv(n).count('\n')) / float(len(text))
            sum += ratio
        ratio = sum / float(len(nodes))
        print(f'Ratio: {ratio:.5}')
        sleep(0.25)


if __name__ == '__main__':
    try:
        run()
    except Exception as e:
        print(e)
        traceback.print_exc()
    finally:
        teardown()
