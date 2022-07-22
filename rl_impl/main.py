from codegame.server_message import ServerMessage
from codegame.client_message import ClientMessage
from stream_wrapper import StreamWrapper
from my_strategy import MyStrategy
from debug_interface import DebugInterface
import socket
import sys
import time

class Runner:
    def __init__(self, host, port, token):
        self.token = token
        self.host = host
        self.port = port

        self.strategy = MyStrategy()

    def run(self):
        while True:
            soc = socket.socket()
            soc.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, True)
            while True:
                try:
                    soc.connect((self.host, self.port))
                except ConnectionRefusedError:
                    time.sleep(1)
                else:
                    break
                finally:
                    print("attempting to connect")

            print("connected")
            socket_stream = soc.makefile('rwb')
            reader = StreamWrapper(socket_stream)
            writer = StreamWrapper(socket_stream)
            writer.write_string(self.token)
            writer.write_int(1)
            writer.write_int(0)
            writer.write_int(1)
            writer.flush()

            debug_interface = DebugInterface(reader, writer)
            while True:
                message = ServerMessage.read_from(reader)
                if isinstance(message, ServerMessage.UpdateConstants):
                    self.strategy.reset(message.constants)
                elif isinstance(message, ServerMessage.GetOrder):
                    order = self.strategy.get_order(message.player_view, debug_interface if message.debug_available else None)
                    ClientMessage.OrderMessage(order).write_to(writer)
                    writer.flush()
                elif isinstance(message, ServerMessage.Finish):
                    self.strategy.finish()
                    break
                elif isinstance(message, ServerMessage.DebugUpdate):
                    self.strategy.debug_update(debug_interface)
                    ClientMessage.DebugUpdateDone().write_to(writer)
                    writer.flush()
                else:
                    raise Exception("Unexpected server message")


if __name__ == "__main__":
    host = "127.0.0.1" if len(sys.argv) < 2 else sys.argv[1]
    port = 31001 if len(sys.argv) < 3 else int(sys.argv[2])
    token = "0000000000000000" if len(sys.argv) < 4 else sys.argv[3]
    Runner(host, port, token).run()
