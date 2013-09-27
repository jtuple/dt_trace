-module(dt_trace).
-compile(export_all).

%% -on_load(init/0).

load() ->
    case code:priv_dir(dt_trace) of
        {error, bad_name} ->
            case code:which(?MODULE) of
                Filename when is_list(Filename) ->
                    Path = filename:join([filename:dirname(Filename),"../priv"]);
                _ ->
                    Path = "../priv"
            end;
        Dir ->
            Path = filename:join(Dir, "dt_trace")
    end,
    case erl_ddll:load(Path, ?MODULE) of
        ok ->
            ok;
        {error, already_loaded} ->
            ok;
        {error, permanent} ->
            ok;
        {error, Error} ->
            error_logger:error_msg("Error loading ~p: ~p~n",
                                   [?MODULE, erl_ddll:format_error(Error)]),
            error
    end.

port() ->
    ok = load(),
    open_port({spawn, "dt_trace"}, [eof]).

trace_on() ->
    erlang:trace(all, true, [call, return_to, {tracer, dt_trace:port()}]),
    erlang:trace_pattern({'_','_','_'}, [{'_', [], [{return_trace}]}], [local]).

trace_off() ->
    erlang:trace(all, false, [call, {tracer, dt_trace:port()}]).

%% erlang:trace(all, true, [all, {tracer, dt_trace:port()}]).
