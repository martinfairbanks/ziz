.code
getAssemblyValue proc
	mov rax, 83
	ret
getAssemblyValue endp

getAssemblyMultiplication proc
	xor rax, rax	; clear rax
	mov ax,5
	mov bx,5
	mul bx		  ; multiply bx with ax, result in ax	
	ret
getAssemblyMultiplication endp

end