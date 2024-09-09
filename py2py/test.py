import json
import os
import subprocess
import sys
import traceback
from os import environ
from time import sleep

processes = []


def teardown():
    for p in processes:
        p.kill()
    for p in processes:
        p.wait()


def happy_path(nodes):
    global processes
    environ['P2P_LOG_LEVEL'] = '1'
    environ['P2P_LOG_PORT'] = '1065'
    log_server = subprocess.Popen(['python', 'server.py'], stdout=subprocess.PIPE, text=True)
    log_parser = subprocess.Popen(['python', 'parser.py'], stdin=log_server.stdout, stdout=subprocess.PIPE, text=True)
    processes = [log_server, log_parser]
    sleep(0.1)
    msg = 'peers,nc'
    for i in range(nodes):
        port = str(8080 + i)
        p2p = subprocess.Popen(
            ['../build/p2p', port, '--logs', '1065'], stdin=subprocess.PIPE, stdout=subprocess.DEVNULL)
        processes.append(p2p)
        msg += f',{port}'
    sleep(0.1)
    nc = subprocess.Popen(['nc', '-N', 'localhost', '8080'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    nc.stdin.write(msg + "\n")
    nc.stdin.close()
    nc.wait()
    return log_parser.stdout


def sample(pipe, secs):
    models = []
    for s in range(secs or 30):
        sleep(1)
        while True:
            line = pipe.readline()
            if not line:
                break
            models.append(json.loads(line))
            print(line)
    return models


def report(models):
    deaths = set()
    errors = []
    total_conn = 0
    max_conn = 0
    known = dict()
    for model in models:
        deaths = deaths.union(model['dead'])
        errors.extend(model['errors'])
        total_conn += sum(len(nodes) for _, nodes in model['graph'].items())
        max_conn = max(max_conn, *(len(nodes) for _, nodes in model['graph'].items()))
        for node, nodes in model['graph'].items():
            known[node] = set(nodes).union(known.get(node, set()))
    len_models = len(models)
    len_nodes = len(models[-1]['graph'])
    avg_conn = float(total_conn) / float(len_models) / float(len_nodes)
    print(f'Models: {len_models}')
    print(f'Nodes: {len_nodes}')
    print(f'Deaths: {deaths if deaths else 'none'}')
    print(f'Total connections: {total_conn}')
    print(f'Max connections: {max_conn}')
    print(f'Avg connections: {avg_conn:.2f}')
    print(f'Min discovered: {min(len(nodes) for _, nodes in known.items())}')
    print(f'Errors: {len(errors)}')
    for err in errors:
        print(err.rstrip())


def run():
    secs = int(sys.argv[1])
    nodes = int(sys.argv[2])
    pipe = happy_path(nodes or 50)
    os.set_blocking(pipe.fileno(), False)
    models = sample(pipe, secs)
    report(models)


if __name__ == '__main__':
    try:
        run()
    except Exception as e:
        print(e)
        traceback.print_exc()
    finally:
        teardown()
