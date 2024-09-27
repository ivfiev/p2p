defmodule Parser do
  use GenServer

  @impl true
  def init(_) do
    {:ok, %{graph: %{}, dead: [], times: %{}, errors: []}}
  end

  @impl true
  def handle_cast({:line, line}, model) do
    new_model = parse_line(model, line)
    {:ok, new_model}
  end

  defp parse_line(model, line) do
    cond do
      String.contains?(line, "ERROR") ->
        %{model | errors: [line | model.errors]}

      String.contains?(line, "[conn:") ->
        node = ""
        nodes = []
        %{model | graph: Map.put(model.graph, node, nodes)}
    end
  end

  defp get_val(key, line) do
    # ix = String.index(line, key)
    # String.
  end
end
