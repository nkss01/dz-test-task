(function() {
    const divUrl = document.getElementById("divUrl");
    const urlInput = document.getElementById("urlInput");
    const urlButton = document.getElementById("urlButton");
    const urlStatus = document.getElementById("urlStatus");
    const divScheme = document.getElementById("divScheme");
    const schemeButton = document.getElementById("schemeButton");
    const schemeStatus = document.getElementById("schemeStatus");
    let _url, _statusLbl, _countStudents;

    const actionElements = {
        list: document.getElementById("divListAction"),
        add: document.getElementById("divAddAction"),
        delete: document.getElementById("divDeleteAction"),
        listButton: document.getElementById("listActionButton"),
        addButton: document.getElementById("addActionButton"),
        deleteButton: document.getElementById("deleteActionButton")
    };

    function getCurrentDateTime() {
        return new Intl.DateTimeFormat('ru-RU', {
            weekday: 'long', year: 'numeric', month: 'long', day: 'numeric',
            hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: false
        }).format(new Date());
    }

    function setStatusLabel(newStatusLabel) {
        _statusLbl = newStatusLabel;
        setStatus(null);
    }

    function setStatus(status, isError) {
        if (!_statusLbl) return;
        _statusLbl.innerText = status || '';
        _statusLbl.hidden = !status;
        _statusLbl.style.color = isError ? "red" : "gray";
    }

    function handleFetchResponse(response, callback) {
        if (!response.ok) {
            return response.json().then(errData => {
                throw new Error(`Ошибка запроса: ${response.status} ${response.statusText} - ${JSON.stringify(errData)}`);
            });
        }
        return response.json();
    }

    function send(body, callback, url = _url) {
        fetch(url, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(body)
        })
        .then(response => handleFetchResponse(response, callback))
        .then(data => callback(data))
        .catch(error => {
            alert("Error sending request");
            console.error(error);
        });
    }

    function countStudents() {
        if (!_countStudents) {
            _countStudents = document.getElementById("countStudents");
        }
        _countStudents.innerText = "";
        send({ method: "countStudents" }, (response) => {
            _countStudents.innerText = `Студентов в базе: ${response.count}`;
        });
    }

    function onConnect(response) {
        if (!response) {
            setStatus("Ошибка соединения", true);
            urlInput.disabled = false;
            urlButton.disabled = false;
            return;
        }
        setStatus("Успешное подключение!");
        _url = urlInput.value;
        divUrl.hidden = true;
        setup();
    }

    function connect() {
        setStatus("Подключение...");
        urlButton.disabled = true;
        urlInput.disabled = true;
        send({}, onConnect, urlInput.value);
    }

    function showStudents(data) {
        console.log(data);
    }

    function handleStudentAction(method, successMessage) {
        return function(response) {
            if (!response) {
                setStatus("Ошибка на стороне сервера", true);
            } else if (response.idNotFound) {
                setStatus("Не найден студент с таким номером", true);
            } else {
                setStatus(successMessage);
                countStudents();
            }
            return;
        }
    }

    function onListAction() {
        setStatusLabel(document.getElementById("listStatus"));
        const listCheckbox = document.getElementById("listCheckbox");
        const listButton = document.getElementById("listButton");

        listButton.addEventListener("click", function() {
            send({
                method: "getStudents",
                showDeleted: listCheckbox.checked,
            }, function(response) {
                const tableBody = document.querySelector('tbody');
                tableBody.innerHTML = '';
				if(!response.data) {
					return;
				}
                response.data.forEach(item => {
                    const row = document.createElement('tr');
                    row.innerHTML = `
                        <td>${item.id}</td>
                        <td>${item.name}</td>
                        <td>${item.surname}</td>
                        <td>${item.middlename}</td>
                        <td>${item.birthdate}</td>
                        <td>${item.group}</td>`;
                    tableBody.appendChild(row);
                });
                setStatus(`Последнее обновление: ${getCurrentDateTime()}`);
            });
        });
    }

    function onAddAction() {
        setStatusLabel(document.getElementById("addStatus"));
        const addButton = document.getElementById("addButton");
        const studentFields = ["nameField", "surnameField", "middlenameField", "birthdateField", "groupField"].map(id => document.getElementById(id));

        addButton.addEventListener("click", function() {
            const emptyField = studentFields.find(field => field.id != "middlenameField" && !field.value);
            if (emptyField) {
                setStatus("Заполните имя, фамилию, дату рождения и группу", true);
                return;
            }
            send({
                method: "createStudent",
                name: studentFields[0].value,
                surname: studentFields[1].value,
                middlename: studentFields[2].value,
                birthdate: studentFields[3].value,
                group: studentFields[4].value,
            }, handleStudentAction("createStudent", "Cтудент успешно добавлен"));
        });
    }

    function onDeleteAction() {
        setStatusLabel(document.getElementById("deleteStatus"));
        const deleteField = document.getElementById("deleteField");
        const deleteButton = document.getElementById("deleteButton");

        deleteButton.addEventListener("click", function() {
            if (!deleteField.value) {
                setStatus("Не заполнен номер", true);
                return;
            }
            send({
                method: "deleteStudent",
                id: Number(deleteField.value),
            }, handleStudentAction("deleteStudent", "Студент успешно удалён"));
        });
    }

    function showActions() {
        document.getElementById("divActions").hidden = false;
        const actionFunctions = {
            list: onListAction,
            add: onAddAction,
            delete: onDeleteAction
        };
        
        actionElements.listButton.addEventListener("click", function() {
            actionFunctions.list();
            actionElements.list.hidden = false;
            actionElements.add.hidden = true;
            actionElements.delete.hidden = true;
        });
        
        actionElements.addButton.addEventListener("click", function() {
            actionFunctions.add();
            actionElements.add.hidden = false;
            actionElements.list.hidden = true;
            actionElements.delete.hidden = true;
        });

        actionElements.deleteButton.addEventListener("click", function() {
            actionFunctions.delete();
            actionElements.delete.hidden = false;
            actionElements.list.hidden = true;
            actionElements.add.hidden = true;
        });
        
        countStudents();
    }

    function setup() {
        send({ method: "checkScheme" }, (response) => {
            if (!response.studentsTableExists) {
                setStatusLabel(schemeStatus);
                schemeButton.addEventListener("click", function() {
                    send({ method: "initStudents" }, (response) => {
                        divScheme.hidden = true;
                        showActions();
                    });
                });
                divScheme.hidden = false;
            } else {
                showActions();
            }
        });
    }

    urlButton.addEventListener("click", connect);
    setStatusLabel(urlStatus);
    urlInput.value = "http://localhost:8080/";

})();