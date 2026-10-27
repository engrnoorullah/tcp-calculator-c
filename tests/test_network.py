import concurrent.futures
import os
import pathlib
import select
import signal
import socket
import subprocess
import unittest

ROOT=pathlib.Path(__file__).resolve().parents[1]

class TCP(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.server=subprocess.Popen([str(ROOT/'build/tcp_server'),'0'],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,start_new_session=True)
        if not select.select([cls.server.stdout],[],[],3)[0]:
            cls.server.kill(); raise RuntimeError('server did not start')
        line=cls.server.stdout.readline().strip()
        if not line.startswith('READY '): raise RuntimeError(line)
        cls.port=int(line.split()[1])

    @classmethod
    def tearDownClass(cls):
        os.killpg(cls.server.pid,signal.SIGTERM)
        cls.server.communicate(timeout=3)

    def request(self,data,fragment=False):
        with socket.create_connection(('127.0.0.1',self.port),timeout=2) as s:
            s.settimeout(2)
            if fragment:
                for b in data: s.sendall(bytes([b]))
            else: s.sendall(data)
            with s.makefile('rb') as f: return f.readline().decode().strip()

    def test_operations(self):
        cases={b'12+30\n':'OK 42',b'-9 * 7\n':'OK -63',b'-9 / 2\n':'OK -4',
               b'2147483647*2147483647\n':'OK 4611686014132420609',
               b'-2147483648 / -1\n':'OK 2147483648'}
        for expression,expected in cases.items():
            with self.subTest(expression=expression): self.assertEqual(self.request(expression),expected)

    def test_bad_input_and_recovery(self):
        for expression in [b'5/0\n',b'abc\n',b'1^2\n',b'2147483648+1\n',b'1+2 garbage\n',b'\n',b'1\0+2\n',b'1'*150+b'\n']:
            with self.subTest(expression=expression): self.assertTrue(self.request(expression).startswith('ERR '))
        self.assertEqual(self.request(b'2+2\n'),'OK 4')

    def test_fragmented_request(self):
        self.assertEqual(self.request(b'100 - 58\n',fragment=True),'OK 42')

    def test_idle_connection_does_not_block_other_clients(self):
        with socket.create_connection(('127.0.0.1',self.port),timeout=2) as idle:
            idle.sendall(b'1+')
            self.assertEqual(self.request(b'6*7\n'),'OK 42')

    def test_parallel_clients(self):
        with concurrent.futures.ThreadPoolExecutor(max_workers=8) as pool:
            results=list(pool.map(lambda _:self.request(b'21*2\n'),range(24)))
        self.assertEqual(results,['OK 42']*24)

    def test_client_and_validation(self):
        result=subprocess.run([str(ROOT/'build/tcp_client'),'127.0.0.1',str(self.port),'7*6'],capture_output=True,text=True,timeout=3)
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertEqual(result.stdout.strip(),'OK 42')
        for port in ['-1','0','65536','abc']:
            result=subprocess.run([str(ROOT/'build/tcp_client'),'127.0.0.1',port,'1+1'],capture_output=True,timeout=3)
            self.assertEqual(result.returncode,1)

    def test_eof_mid_request(self):
        with socket.create_connection(('127.0.0.1',self.port),timeout=2) as s:
            s.sendall(b'1+');s.shutdown(socket.SHUT_WR)
            self.assertTrue(s.recv(160).startswith(b'ERR incomplete request'))

if __name__=='__main__': unittest.main()
