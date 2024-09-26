defmodule SocketServer do
  @port 1065

  def start() do
    {:ok, socket} =
      :gen_tcp.listen(@port, [:binary, packet: :raw, active: false, reuseaddr: true])

    server_loop(socket)
  end

  defp server_loop(listener) do
    accept(listener)
    server_loop(listener)
  end

  defp accept(socket) do
    {:ok, client} = :gen_tcp.accept(socket)
    spawn(fn -> client_loop(client) end)
  end

  defp client_loop(client) do
    case :gen_tcp.recv(client, 0) do
      {:ok, data} ->
        IO.puts("Received: [#{data}]")
        client_loop(client)

      {:error, err} ->
        {
          IO.puts("Connection closed: #{err}")
        }
    end
  end
end
