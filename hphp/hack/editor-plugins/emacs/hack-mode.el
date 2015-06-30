;;; -*- lexical-binding: t -*-
;;; hack-mode.el --- Major mode for the Hack programming language

;;; Commentary:
;; 
;; Implements `hack-mode' for the Hack programming language.  This includes
;; basic support for Hack typechecker integration and the Hack autocompletion
;; service
;;
;; Indentation is currently provided solely by the CC indentation engine.
;; It does not handle indentation or highlighting of XHP expressions.

;;; Code:
(require 'icicles nil t)
(require 'auto-complete nil t)

(require 'font-lock)
(require 'cc-mode)
(require 'cc-langs)
(eval-when-compile
  (require 'regexp-opt))

(defgroup hack nil
  "Major mode `hack-mode' for editing Hack code."
  :prefix "hack-"
  :group 'languages)

(defface hack-default
  '((default
	  (:inherit default)))
  "Default face in `hack-mode' buffers."
  :group 'hack)

(defface hack-dollar
  '((default
	  (:inherit hack-default)))
  "Face for the dollar in variable names"
  :group 'hack)

(defface hack-constant
  '((default
	  (:inherit (font-lock-constant-face hack-default))))
  "Face for constants"
  :group 'hack)

(defface hack-variable-name
  '((default
	  (:inherit (font-lock-variable-name-face hack-default))))
  "Face for variable names"
  :group 'hack)

(defface hack-keyword
  '((default
	  (:inherit (font-lock-keyword-face hack-default))))
  "Face for Hack keywords"
  :group 'hack)

(defface hack-attribute
  '((default
	  (:inherit hack-keyword)))
  "Face for method and variable attributes"
  :group 'hack)

(defface hack-type
  '((default
	  (:inherit (font-lock-type-face hack-default))))
  "Face for Hack type names"
  :group 'hack)

(defface hack-function-name
  '((default
	  (:inherit (font-lock-function-name-face hack-default))))
  "Face for function names"
  :group 'hack)

(defface hack-special
  '((default
	  (:inherit (font-lock-builtin-face hack-default))))
  "Face for special Hack names (__construct, __toString, etc)"
  :group 'hack)

(defface hack-builtin
  '((default
	  (:inherit hack-special)))
  "Face for built-in hack functions"
  :group 'hack)

(defface hack-field-name
  '((default
	  (:inherit (font-lock-variable-name-face hack-default))))
  "Face for field names in expressions like $foo->bar"
  :group 'hack)

(defface hack-function-call
  '((default
	  (:inherit hack-function-name)))
  "Face for functions call expressions"
  :group 'hack)

(defface hack-method-call
  '((default
	  (:inherit hack-function-call)))
  "Face for method call expressions"
  :group 'hack)

(defconst hack-keywords
  (eval-when-compile
	(regexp-opt
	 '("exit" "die" "const" "return" "yield" "try" "catch" "finally"
	   "throw" "if" "else" "while" "do" "for" "foreach" "instanceof"
	   "as" "switch" "default" "goto" "attribute" "category"
	   "children" "enum" "clone" "include" "include_once" "require"
	   "require_once" "namespace" "use" "global" "await")))
  "Hack Keywords.")

(defconst hack-builtins
  (eval-when-compile
	(regexp-opt
	 '("echo" "tuple" "list" "empty" "isset" "unset")))
  "Hack builtins.")

(defconst hack-type-regexp
  "\\(?::\\|\\<\\)[a-z_][a-z0-9_:]*\\>")

(defconst hack-types
  (eval-when-compile
    (regexp-opt '("array" "bool" "char" "float" "int" "mixed" "string" "void")))
  "Hack types.")

(defconst hack-special-methods
  (eval-when-compile
	(regexp-opt '("__construct" "__destruct" "__toString" "__clone" "__sleep" "__wakeup")))
  "Special Hack methods.")

(defconst hack-font-lock-keywords-1
  (list
	;; Keywords
   (cons
	(concat "[^_$]?\\<\\(" hack-keywords "\\)\\>[^_]?")
	'(1 'hack-keyword))

	;; Builtins
   (cons
	(concat "[^_$]?\\<\\(" hack-builtins "\\)\\>[^_]?")
	'(1 'hack-builtin))
   
   '("\\<\\(break\\|case\\|continue\\)\\>\\s-+\\(-?\\sw+\\)?"
	 (1 'hack-keyword) (2 'hack-constant t t))
   '("^\\s-*\\(\\sw+\\):\\>" (1 'hack-constant nil t))

   ;; PHP/Hack Tag including mode header
   '("<\\?\\(?:php\\|hh\\)\\s-*?" (0 font-lock-preprocessor-face)
	 ("//\\s-+\\(partial\\|decl\\|strict\\)" nil nil (1 font-lock-warning-face t t))))
   "Minimal highlighting for Hack mode.")

(defconst hack-font-lock-keywords-2
  (append
   hack-font-lock-keywords-1
   (list
   ;; Type declarations
   '("\\<\\(class\\|interface\\|trait\\|type\\|newtype\\)\\s-+\\(:?\\sw+\\)?"
	 (1 'hack-keyword) (2 'hack-type nil t))
   ;; Tokens following certain keywords are known to be types
   `("\\<\\(new\\|extends\\)\\s-+" (1 'hack-keyword)
	 (,hack-type-regexp nil nil (0 'hack-type nil t)))
   ;; implements takes a list of types, handle it separately
   `("\\<\\(implements\\)\\s-+\\$?" (1 'hack-keyword t)
	 (,hack-type-regexp nil nil (0 'hack-type- nil t)))
   ;; async must come before function keyword
   '("\\<\\(\\(?:async\\s-+\\)?function\\)\\s-*&?\\(\\sw+\\)?\\s-*("
      (1 'hack-keyword)
      (2 'hack-function-name nil t))
   '("\\(?:[^$]\\|^\\)\\<\\(self\\|parent\\|static\\)\\>" (1 'hack-special nil nil))
   ;; method and variable attributes
   '("\\<\\(private\\|protected\\|public\\|static\\)\\s-+\\$?\\sw+"
	 (1 'hack-attribute t t))
   ;; method attributes
   '("\\<\\(abstract\\|final\\)\\s-+\\sw+"
	 (1 'hack-attribute t t))

   (cons
	(concat "[^_$]?\\<\\(" hack-types "\\)\\>[^_]?")
	'(1 'hack-type))
	))
  "Medium highlighting for Hack mode.")

(defconst hack-font-lock-keywords-3
  (append
   hack-font-lock-keywords-2
   (list
	'("\\<\\($\\)\\sw+\\>" (1 'hack-dollar))
	'("\\$\\(this\\)" (1 'hack-special))
	'("\\$\\(\\sw+\\)" (1 'hack-variable-name t))
	'("\\<[0-9]+" . 'hack-constant)
	'("->\\(\\sw+\\)" (1 'hack-field-name t t))
	'("->\\(\\sw+\\)\\s-*(" (1 'hack-method-call t t))
	'("\\<\\([a-z\\_][a-z0-9\\_]*\\)\\s-*[[(]" (1 'hack-function-call))
	
	;; Highlight types where they are easy to detect
	;; Return types
	`(")\\s-*:\\s-*" (,hack-type-regexp nil nil (0 'hack-type nil t)))
	
	;; Highlight special methods
	(cons
	 (concat "\\<function\\s-+\\(" hack-special-methods "\\)(")
	 '(1 'hack-special t t))
	))
  "Full highlighting for Hack mode.")

(defconst hack-block-stmt-1-kwds '("do" "else" "finally" "try"))
(defconst hack-block-stmt-2-kwds
  '("for" "if" "while" "switch" "foreach"))

(defconst hack-block-stmt-1-key
  (regexp-opt hack-block-stmt-1-kwds))
(defconst hack-block-stmt-2-key
  (regexp-opt hack-block-stmt-2-kwds))

(defconst hack-class-decl-kwds '("class" "interface" "trait"))

(defconst hack-class-key
  (concat
   "\\(" (regexp-opt hack-class-decl-kwds) "\\)\\s-+"
   (c-lang-const c-symbol-key c)                ;; Class name.
   "\\(\\s-+extends\\s-+" (c-lang-const c-symbol-key c) "\\)?" ;; Name of superclass.
   "\\(\\s-+implements\\s-+[^{]+{\\)?")) ;; List of any adopted protocols.

(defconst hack-client-binary "hh_client"
  "Hack client binary.")

(defvar hack-mode-abbrev-table nil)

(defun hack-get-completions (&optional prefix)
  "Symbol completion function for `hack-mode'."
  (when (executable-find hack-client-binary)
	(let* ((cur-buf (current-buffer)) ;; Save the current buffer
		   (cur-point (point)) ;; Save the current point
		   ;; Get the bounds of the symbol we're autocompleting
		   (cur-symbol-bounds (if prefix
								  (cons (- (point) (length prefix)) (point))
								(bounds-of-thing-at-point 'symbol)))
		   (complete-start (or (car cur-symbol-bounds) cur-point))
		   (complete-end (or (cdr cur-symbol-bounds) cur-point))
		   (candidates nil))
	  ;; Use a temporary buffer for creating the input text and for storing the output
	  (with-temp-buffer
		;; Copy from the beginning of the buffer to the position of the point
		;; Remove the properties, though I don't think it actually matters too much
		(insert-buffer-substring-no-properties cur-buf 1 cur-point)
		;; Insert the autocomplete marker
		(insert "AUTO332")
		;; Insert the rest of the buffer
		(insert-buffer-substring-no-properties cur-buf cur-point)
		;; Call the hh_client binary, the input text is piped in through stdin, then deleted from the buffer.
		;; The output then goes into the same temporary buffer
		(call-process-region 1 (buffer-size) hack-client-binary t t nil "--auto-complete")
		;; Check that there were any valid completions
		(unless (= 0 (count-lines 1 (buffer-size)))
		  ;; Put the point at the start of the buffer
		  (goto-char (point-min))
		  (let ((cur-candidate-symbol nil)
				(cur-candidate-type nil))
			;; Each autocomplete candidate looks like this:
			;;    <symbol> <type>
			;; one per line
			(while (not (eobp))
			  ;; Grab the symbol at the beginning of the list
			  (setq cur-candidate-symbol (thing-at-point 'symbol))
			  (when cur-candidate-symbol
				;; Skip over the symbol name
				(forward-char (1+ (length cur-candidate-symbol)))
				(setq cur-candidate-type (buffer-substring-no-properties (point) (line-end-position)))
				(setq cur-candidate-symbol (propertize cur-candidate-symbol
													   'ac-hack-type cur-candidate-type))
				;; Add it to the list
				(push cur-candidate-symbol candidates))
			  (forward-line)))))
	  (when candidates
		(list complete-start complete-end candidates)))))

(defun hack-completion ()
  (let ((candidates (hack-get-completions)))
	(when candidates
	(append candidates
			(list
			 :annotation-function (lambda (candidate)
									(concat " " (get-text-property 0 'ac-hack-type candidate))))))))

;;;###autoload
(define-derived-mode hack-mode c-mode "Hack"
  "A major mode for Hack files\n\n\\{hack-mode-map}"
  (c-add-language 'hack-mode 'c-mode)

  (c-initialize-cc-mode t)
  (c-init-language-vars hack-mode)
  (c-common-init 'hack-mode)

  (setq-local c-opt-cpp-start "<\\?\\(?:php\\|hh\\)\\s-*?")
  (setq-local c-opt-cpp-prefix "<\\?\\(?:php\\|hh\\)\\s-*?")

  (c-set-offset 'cpp-macro 0)

  (setq-local c-block-stmt-1-key hack-block-stmt-1-key)
  (setq-local c-block-stmt-2-key hack-block-stmt-2-key)
  
  (setq-local c-class-key hack-class-key)

  (setq-local font-lock-defaults
			  '((hack-font-lock-keywords-1
				 hack-font-lock-keywords-2
				 hack-font-lock-keywords-3)
				nil
				t
				(("_" . "w")
				 ("'" . "\"")
				 ("`" . "\"")
				 ("$" . ".")
				 ("#" . "< b")
				 ("/" . ". 124b")
				 ("*" . ". 23")
				 (?\n . "> b"))
				nil))
  
  (setq font-lock-maximum-decoration t)
  (setq case-fold-search t)
  
  (setq-local compile-command (concat hack-client-binary " --from emacs"))
  
  (add-hook 'completion-at-point-functions 'hack-completion nil t) 

  (run-hooks 'hack-mode-hooks))

(when (featurep 'auto-complete)
  (defun ac-hack-candidate ()
	(save-restriction
	  (widen)
	  (nth 2 (hack-get-completions ac-prefix))))
  
  (defun ac-hack-prefix ()
	(or (ac-prefix-symbol)
		(let ((c (char-before)))
		  (when (or
				 ;; ->
				 (and (eq ?> c)
					  (eq ?- (char-before (1- (point)))))
				 ;; ::
				 (and (eq ?: c)
					  (eq ?: (char-before (1- (point))))))
			(point)))))

  (ac-define-source hack
	'((candidates . ac-hack-candidate)
	  (prefix . ac-hack-prefix)
	  (requires . 0)))

  (add-hook 'hack-mode-hook (lambda () (setq ac-sources '(ac-source-hack))))
)

(provide 'hack-mode)

;;; hack-mode.el ends here
