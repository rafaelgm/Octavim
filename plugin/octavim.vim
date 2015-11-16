" Site...: https://github.com/rafaelgm/Octavim
" Author.: Rafael Monteiro

let s:firstRun = 1
let s:octaveWindow = "Octave"
let s:pathToOctaveHelper = expand('<sfile>:p:h:h') . '\tools\OctaveHelper.dll'

let s:block = {}

function! CallOctaveHelper(cmd)
    if filereadable(s:pathToOctaveHelper)
        exec libcall(s:pathToOctaveHelper, 'OctaveHelper', a:cmd)
    else
        echo 'Octavim: main library not compiled (see instructions on https://github.com/rafaelgm/Octavim)'
    endif
endfunction

function! GetCells()
    " '1' is the signal to call GetCells in the DLL
    call CallOctaveHelper('1 ' . line('.') . " ". join(getline(1, line('$')), "\n"))
endfunction

function! GoToPreviousCell()
    if s:block != {}
        execute ':' . s:block.prev
    endif
endfunction

function! GoToNextCell()
    if s:block != {}
        execute ':' . (s:block.end + 1)
    endif
endfunction

function! HighlightCell()
    if s:firstRun == 0
        exec 'syntax clear octaveCell'
    endif
    let s:firstRun = 0

    call GetCells()

    highlight octaveCell guibg=#2A2520
    if s:block != {}
        exec 'match octaveCell /\%>' . (max([s:block.startExt, line('w0')]) - 1) . 'l\%<' . s:block.endExt . 'l/'
    else
        exec 'match octaveCell /\%>0l\%<0l/'
    endif
endfunction

function! Run(cmd)
    " '2' is the signal to call Run in the DLL
    call CallOctaveHelper('2 ' . s:octaveWindow . "\n" . join(a:cmd, "\n") . "\n")
endfunction

function! RunCell()
    if s:block != {}
        call Run(getline(s:block.startExt, s:block.endExt - 1))
    endif
endfunction

function! RunFile()
    if s:block != {}
        call Run(getline(1, line('$')))
    endif
endfunction

" Credits: http://stackoverflow.com/a/6271254
function! s:get_visual_selection()
    " Why is this not a built-in Vim script function?!
    let [lnum1, col1] = getpos("'<")[1:2]
    let [lnum2, col2] = getpos("'>")[1:2]
    let lines = getline(lnum1, lnum2)
    let lines[-1] = lines[-1][: col2 - (&selection == 'inclusive' ? 1 : 2)]
    let lines[0] = lines[0][col1 - 1:]
    return lines
endfunction

function! RunSelection() range
    call Run(s:get_visual_selection())
endfunction

nnoremap <silent> <f5>          :call RunFile()          <cr>
inoremap <silent> <f5>          :call RunFile()          <cr>
vnoremap <silent> <f5>          :call RunFile()          <cr>
vnoremap <silent> <buffer> <f9> :call RunSelection()     <cr>
nnoremap <silent> <c-cr>        :call RunCell()          <cr>
inoremap <silent> <c-cr> <esc>  :call RunCell()          <cr>i
nnoremap <silent> <c-k>         :call GoToPreviousCell() <cr>
nnoremap <silent> <c-j>         :call GoToNextCell()     <cr>
au CursorMoved,CursorMovedI *.m call HighlightCell()
"autocmd TextChanged,TextChangedI *.m call GetCells()
