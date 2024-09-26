defmodule P2p do
  def start(port \\ 8080) do
    Port.open(
      {:spawn_executable, "../build/p2p"},
      [
        {:args, [to_charlist(port)]},
        {:env,
         [
           {'P2P_LOG_LEVEL', '1'},
           {'P2P_LOG_PORT', '1065'}
         ]}
      ]
    )
  end

  def kill_all() do
    Port.open({:spawn, "pkill p2p"}, [])
  end

  def start_many(n) do
    Enum.each(0..(n - 1), fn i -> start(8080 + i) end)
  end
end
